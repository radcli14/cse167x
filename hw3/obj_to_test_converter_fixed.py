#!/usr/bin/env python3
"""
OBJ to Test File Converter
Converts Wavefront .obj files to the .test format used by the hw3 raytracer.

WORKING CAMERA AND LIGHTING SETTINGS FOR DC.OBJ:
================================================
Camera: 0.0 0.0 0.0 -2.0 -1.5 -2.0 0.0 1.0 0.0 45.0
- Position: (0, 0, 0) - at origin
- Look-at: (-2.0, -1.5, -2.0) - looking at the bottom-left corner of model bounding box
- Up: (0, 1, 0) - Y-up convention
- FOV: 45 degrees

Lights:
- Light 1: point -0.42 -0.38 -0.44 1 1 1 (30W equivalent)
- Light 2: point -1.99 -1.49 -1.99 0.33 0.33 0.33 (10W equivalent)

Model Bounding Box: min=[-2.0, -1.5, -2.0], max=[0.0, 0.5, 0.0]
Model Center: [-1.0, -0.5, -1.0]

COORDINATE CONVERSION NOTES:
===========================
Blender (Z-up) to Y-up conversion:
- X_blender → X_yup (unchanged)
- Y_blender → Z_yup 
- Z_blender → Y_yup

MTL FILE INTERPRETATION:
=======================
- Ka: Ambient reflectance coefficient (NOT ambient color)
- Kd: Diffuse reflectance coefficient (this is the actual color)
- Ks: Specular reflectance coefficient
- Ke: Emission color
- Ns: Shininess/specular exponent

Usage:
    python obj_to_test_converter_fixed.py input.obj output.test [options]

Options:
    --camera x y z lookat_x lookat_y lookat_z up_x up_y up_z fov
    --size width height
    --output filename
    --maxdepth depth
    --material ambient_r ambient_g ambient_b diffuse_r diffuse_g diffuse_b specular_r specular_g specular_b shininess emission_r emission_g emission_b
    --light point x y z r g b
    --light directional x y z r g b
    --scale x y z
    --translate x y z
    --rotate x y z angle
"""

import argparse
import sys
import os
from typing import List, Tuple, Optional
import math

class OBJConverter:
    def __init__(self):
        self.vertices = []
        self.normals = []
        self.faces = []
        self.materials = {}
        self.current_material = None
        self.material_library = {}
        self.material_assignments = []  # List of (face_indices, material_name) tuples
        
    def parse_mtl(self, filename: str) -> bool:
        """Parse an MTL file and extract material definitions."""
        try:
            current_material = None
            with open(filename, 'r') as f:
                for line_num, line in enumerate(f, 1):
                    line = line.strip()
                    if not line or line.startswith('#'):
                        continue
                        
                    parts = line.split()
                    if not parts:
                        continue
                        
                    command = parts[0].lower()
                    
                    if command == 'newmtl':  # new material
                        if len(parts) >= 2:
                            current_material = parts[1]
                            self.material_library[current_material] = {
                                'ambient': [0.2, 0.2, 0.2],
                                'diffuse': [0.8, 0.8, 0.8],
                                'specular': [0.0, 0.0, 0.0],
                                'shininess': 0.0,
                                'emission': [0.0, 0.0, 0.0]
                            }
                        else:
                            print(f"Warning: Invalid material name at line {line_num}")
                            
                    elif current_material and command in ['ka', 'kd', 'ks', 'ke']:  # material properties
                        if len(parts) >= 4:
                            try:
                                r, g, b = float(parts[1]), float(parts[2]), float(parts[3])
                                if command == 'ka':  # ambient reflectance coefficient
                                    # Ka is a reflectance coefficient, not a color
                                    # For raytracer, we'll use a small ambient value
                                    # and let the diffuse color dominate
                                    self.material_library[current_material]['ambient'] = [0.1, 0.1, 0.1]
                                elif command == 'kd':  # diffuse reflectance coefficient
                                    # Kd is the diffuse color/reflectance
                                    self.material_library[current_material]['diffuse'] = [r, g, b]
                                elif command == 'ks':  # specular reflectance coefficient
                                    # Ks is the specular reflectance
                                    self.material_library[current_material]['specular'] = [r, g, b]
                                elif command == 'ke':  # emission color
                                    # Ke is actually an emission color
                                    self.material_library[current_material]['emission'] = [r, g, b]
                            except ValueError:
                                print(f"Warning: Invalid color values at line {line_num}")
                                
                    elif current_material and command == 'ns':  # shininess
                        if len(parts) >= 2:
                            try:
                                shininess = float(parts[1])
                                self.material_library[current_material]['shininess'] = shininess
                            except ValueError:
                                print(f"Warning: Invalid shininess value at line {line_num}")
                                
            print(f"Parsed {len(self.material_library)} materials from {filename}")
            return True
            
        except FileNotFoundError:
            print(f"Warning: Material library '{filename}' not found")
            return False
        except Exception as e:
            print(f"Error parsing MTL file: {e}")
            return False
    
    def parse_obj(self, filename: str) -> bool:
        """Parse an OBJ file and extract vertices, normals, and faces."""
        try:
            face_start_index = 0
            with open(filename, 'r') as f:
                for line_num, line in enumerate(f, 1):
                    line = line.strip()
                    if not line or line.startswith('#'):
                        continue
                        
                    parts = line.split()
                    if not parts:
                        continue
                        
                    command = parts[0].lower()
                    
                    if command == 'v':  # vertex
                        if len(parts) >= 4:
                            x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
                            self.vertices.append((x, y, z))
                        else:
                            print(f"Warning: Invalid vertex at line {line_num}")
                            
                    elif command == 'vn':  # vertex normal
                        if len(parts) >= 4:
                            nx, ny, nz = float(parts[1]), float(parts[2]), float(parts[3])
                            self.normals.append((nx, ny, nz))
                        else:
                            print(f"Warning: Invalid normal at line {line_num}")
                            
                    elif command == 'f':  # face
                        if len(parts) >= 4:
                            face = []
                            for part in parts[1:]:
                                # Handle different face formats: v, v/vt, v//vn, v/vt/vn
                                vertex_parts = part.split('/')
                                if len(vertex_parts) >= 1:
                                    try:
                                        vertex_idx = int(vertex_parts[0]) - 1  # OBJ indices are 1-based
                                        face.append(vertex_idx)
                                    except ValueError:
                                        print(f"Warning: Invalid face vertex at line {line_num}")
                                        continue
                            if len(face) >= 3:
                                # Triangulate if necessary
                                for i in range(1, len(face) - 1):
                                    self.faces.append((face[0], face[i], face[i + 1]))
                        else:
                            print(f"Warning: Invalid face at line {line_num}")
                            
                    elif command == 'usemtl':  # material
                        if len(parts) >= 2:
                            # Record the material assignment for the current face range
                            if self.current_material and face_start_index < len(self.faces):
                                self.material_assignments.append((face_start_index, len(self.faces), self.current_material))
                            self.current_material = parts[1]
                            face_start_index = len(self.faces)
                            
                    elif command == 'mtllib':  # material library
                        if len(parts) >= 2:
                            mtl_filename = parts[1]
                            # Try to find the .mtl file in the same directory as the .obj file
                            import os
                            obj_dir = os.path.dirname(filename)
                            mtl_path = os.path.join(obj_dir, mtl_filename)
                            if os.path.exists(mtl_path):
                                self.parse_mtl(mtl_path)
                            else:
                                print(f"Warning: Material library '{mtl_path}' not found")
                        
            # Record the final material assignment
            if self.current_material and face_start_index < len(self.faces):
                self.material_assignments.append((face_start_index, len(self.faces), self.current_material))
                        
            print(f"Parsed {len(self.vertices)} vertices, {len(self.normals)} normals, {len(self.faces)} faces")
            print(f"Material assignments: {len(self.material_assignments)} groups")
            return True
            
        except FileNotFoundError:
            print(f"Error: File '{filename}' not found")
            return False
        except Exception as e:
            print(f"Error parsing OBJ file: {e}")
            return False
    
    def write_test_file(self, filename: str, options: dict) -> bool:
        """Write the parsed data to a .test file."""
        try:
            with open(filename, 'w') as f:
                # Write header
                f.write(f"# Converted from OBJ file\n")
                f.write(f"# Generated by obj_to_test_converter_fixed.py\n\n")
                
                # Write general settings
                f.write(f"size {options.get('width', 640)} {options.get('height', 480)}\n")
                f.write(f"output {options.get('output', 'converted.obj.png')}\n")
                f.write(f"maxdepth {options.get('maxdepth', 5)}\n\n")
                
                # Write camera
                camera = options.get('camera', [0, 0, 5, 0, 0, 0, 0, 1, 0, 45])
                f.write(f"camera {' '.join(map(str, camera))}\n\n")
                
                # Write lights
                lights = options.get('lights', [])
                if not lights:
                    # Default light if none specified
                    f.write(f"point 5 5 5 1 1 1\n")
                    f.write(f"ambient 0.2 0.2 0.2\n\n")
                else:
                    for light in lights:
                        f.write(f"{light}\n")
                    f.write("\n")
                
                # Write vertices
                if self.vertices:
                    f.write(f"maxverts {len(self.vertices)}\n\n")
                    
                    for x, y, z in self.vertices:
                        f.write(f"vertex {x} {y} {z}\n")
                    f.write("\n")
                    
                    # Write faces as triangles with material assignments
                    if self.faces:
                        f.write("# Faces converted to triangles with materials\n")
                        
                        # Apply transformations if specified
                        transform = options.get('transform', [])
                        if transform:
                            f.write("pushTransform\n")
                            for cmd in transform:
                                f.write(f"{cmd}\n")
                        
                        # Write faces grouped by material
                        if self.material_assignments:
                            for start_idx, end_idx, material_name in self.material_assignments:
                                # Write material properties for this group
                                if material_name in self.material_library:
                                    mat = self.material_library[material_name]
                                    f.write(f"# Material: {material_name}\n")
                                    f.write(f"ambient {mat['ambient'][0]} {mat['ambient'][1]} {mat['ambient'][2]}\n")
                                    f.write(f"diffuse {mat['diffuse'][0]} {mat['diffuse'][1]} {mat['diffuse'][2]}\n")
                                    f.write(f"specular {mat['specular'][0]} {mat['specular'][1]} {mat['specular'][2]}\n")
                                    f.write(f"shininess {mat['shininess']}\n")
                                    f.write(f"emission {mat['emission'][0]} {mat['emission'][1]} {mat['emission'][2]}\n\n")
                                else:
                                    # Use default material if not found
                                    material = options.get('material', [0.2, 0.2, 0.2, 0.8, 0.8, 0.8, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
                                    f.write(f"# Default material for {material_name}\n")
                                    f.write(f"ambient {material[0]} {material[1]} {material[2]}\n")
                                    f.write(f"diffuse {material[3]} {material[4]} {material[5]}\n")
                                    f.write(f"specular {material[6]} {material[7]} {material[8]}\n")
                                    f.write(f"shininess {material[9]}\n")
                                    f.write(f"emission {material[10]} {material[11]} {material[12]}\n\n")
                                
                                # Write faces for this material group
                                for i in range(start_idx, end_idx):
                                    if i < len(self.faces):
                                        face = self.faces[i]
                                        if len(face) == 3:
                                            f.write(f"tri {face[0]} {face[1]} {face[2]}\n")
                        else:
                            # No material assignments, use default material
                            material = options.get('material', [0.2, 0.2, 0.2, 0.8, 0.8, 0.8, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
                            f.write(f"ambient {material[0]} {material[1]} {material[2]}\n")
                            f.write(f"diffuse {material[3]} {material[4]} {material[5]}\n")
                            f.write(f"specular {material[6]} {material[7]} {material[8]}\n")
                            f.write(f"shininess {material[9]}\n")
                            f.write(f"emission {material[10]} {material[11]} {material[12]}\n\n")
                            
                            for face in self.faces:
                                if len(face) == 3:
                                    f.write(f"tri {face[0]} {face[1]} {face[2]}\n")
                        
                        if transform:
                            f.write("popTransform\n")
                        f.write("\n")
                
            print(f"Successfully wrote test file: {filename}")
            return True
            
        except Exception as e:
            print(f"Error writing test file: {e}")
            return False
    
    def get_bounding_box(self) -> Tuple[List[float], List[float]]:
        """Calculate the bounding box of the model."""
        if not self.vertices:
            return ([0, 0, 0], [0, 0, 0])
        
        min_coords = [float('inf')] * 3
        max_coords = [float('-inf')] * 3
        
        for x, y, z in self.vertices:
            min_coords[0] = min(min_coords[0], x)
            min_coords[1] = min(min_coords[1], y)
            min_coords[2] = min(min_coords[2], z)
            max_coords[0] = max(max_coords[0], x)
            max_coords[1] = max(max_coords[1], y)
            max_coords[2] = max(max_coords[2], z)
        
        return min_coords, max_coords
    
    def suggest_camera_position(self) -> List[float]:
        """Suggest a camera position based on the model's bounding box."""
        min_coords, max_coords = self.get_bounding_box()
        
        # Calculate center and size
        center = [(min_coords[i] + max_coords[i]) / 2 for i in range(3)]
        size = max(max_coords[i] - min_coords[i] for i in range(3))
        
        # Position camera at a reasonable distance
        distance = size * 2.0
        
        # Camera position: slightly elevated and back
        camera_pos = [center[0], center[1] - distance * 0.5, center[2] + distance]
        look_at = center
        up = [0, 1, 0]
        fov = 45
        
        return camera_pos + look_at + up + [fov]

def main():
    parser = argparse.ArgumentParser(
        description="Convert OBJ files to .test format for hw3 raytracer",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument('input', help='Input OBJ file')
    parser.add_argument('output', help='Output .test file')
    
    # General options
    parser.add_argument('--size', nargs=2, type=int, default=[640, 480],
                       help='Image size (width height)')
    parser.add_argument('--output-file', dest='output_filename', default='converted.obj.png',
                       help='Output image filename')
    parser.add_argument('--maxdepth', type=int, default=5,
                       help='Maximum ray depth')
    
    # Camera options
    parser.add_argument('--camera', nargs=10, type=float,
                       help='Camera parameters: x y z lookat_x lookat_y lookat_z up_x up_y up_z fov')
    parser.add_argument('--auto-camera', action='store_true',
                       help='Automatically position camera based on model bounds')
    
    # Material options
    parser.add_argument('--material', nargs=13, type=float,
                       default=[0.2, 0.2, 0.2, 0.8, 0.8, 0.8, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
                       help='Material: ambient_r g b diffuse_r g b specular_r g b shininess emission_r g b')
    
    # Light options
    parser.add_argument('--light', action='append', nargs=7,
                       help='Add light: type x y z r g b (type: point or directional)')
    
    # Transform options
    parser.add_argument('--scale', nargs=3, type=float,
                       help='Scale model by x y z')
    parser.add_argument('--translate', nargs=3, type=float,
                       help='Translate model by x y z')
    parser.add_argument('--rotate', nargs=4, type=float,
                       help='Rotate model: x y z angle_degrees')
    
    args = parser.parse_args()
    
    # Check input file
    if not os.path.exists(args.input):
        print(f"Error: Input file '{args.input}' not found")
        sys.exit(1)
    
    # Create converter and parse OBJ
    converter = OBJConverter()
    if not converter.parse_obj(args.input):
        sys.exit(1)
    
    # Prepare options
    options = {
        'width': args.size[0],
        'height': args.size[1],
        'output': args.output_filename,
        'maxdepth': args.maxdepth,
        'material': args.material,
        'lights': [],
        'transform': []
    }
    
    # Handle camera
    if args.camera:
        options['camera'] = args.camera
    elif args.auto_camera:
        options['camera'] = converter.suggest_camera_position()
    else:
        # Default camera
        options['camera'] = [0, 0, 5, 0, 0, 0, 0, 1, 0, 45]
    
    # Handle lights
    if args.light:
        for light in args.light:
            light_type, x, y, z, r, g, b = light
            options['lights'].append(f"{light_type} {x} {y} {z} {r} {g} {b}")
    
    # Handle transforms
    if args.scale:
        options['transform'].append(f"scale {' '.join(map(str, args.scale))}")
    if args.translate:
        options['transform'].append(f"translate {' '.join(map(str, args.translate))}")
    if args.rotate:
        x, y, z, angle = args.rotate
        options['transform'].append(f"rotate {x} {y} {z} {angle}")
    
    # Write test file
    if not converter.write_test_file(args.output, options):
        sys.exit(1)
    
    # Print summary
    print(f"\nConversion Summary:")
    print(f"  Input: {args.input}")
    print(f"  Output: {args.output}")
    print(f"  Vertices: {len(converter.vertices)}")
    print(f"  Faces: {len(converter.faces)}")
    print(f"  Image size: {options['width']}x{options['height']}")
    print(f"  Camera: {' '.join(map(str, options['camera']))}")
    
    if options['transform']:
        print(f"  Transforms: {len(options['transform'])} applied")

if __name__ == '__main__':
    main() 