import sys
import os
import math
import FreeCAD
import Part

cwd = os.getcwd()

print("--- Generating CASING_NANO (FreeCAD 1.1.4) with 4.0mm Base (Nut 2.5mm + 1.5mm Ceiling) ---")

def make_corner_cut(x, y, R, dx_dir, dy_dir):
    bx = x if dx_dir > 0 else x - R
    by = y - R if dy_dir < 0 else y
    box = Part.makeBox(R, R, 30, FreeCAD.Vector(bx, by, -5))
    center = FreeCAD.Vector(x + dx_dir * R, y + dy_dir * R, -5)
    cyl = Part.makeCylinder(R, 30, center, FreeCAD.Vector(0, 0, 1))
    return box.cut(cyl)

def make_hex_prism(s, height, center_x, center_y, z_bottom):
    R = s / math.sqrt(3.0)
    pts = []
    for i in range(6):
        angle = math.radians(60 * i + 30)
        pts.append(FreeCAD.Vector(center_x + R * math.cos(angle), center_y + R * math.sin(angle), z_bottom))
    pts.append(pts[0])
    wire = Part.makePolygon(pts)
    face = Part.Face(wire)
    prism = face.extrude(FreeCAD.Vector(0, 0, height))
    return prism

R_fillet = 3.0
corner_cuts = [
    make_corner_cut(0.0, 0.0, R_fillet, 1, -1),
    make_corner_cut(0.0, -73.0, R_fillet, 1, 1),
    make_corner_cut(60.0, 0.0, R_fillet, -1, -1),
    make_corner_cut(60.0, -73.0, R_fillet, -1, 1),
]

# Total base thickness = 4.0 mm (M3 nut 2.4-2.5 mm + 1.5 mm ceiling)
base_thick = 4.0
c = 0.30  # 0.30 mm clearance

# 1. ATAS
body = Part.makeBox(36.0, 73.0, 24.0, FreeCAD.Vector(12.0, -73.0, 0.0))
left_wing = Part.makeBox(12.0, 73.0, 1.5, FreeCAD.Vector(0.0, -73.0, 0.0))
right_wing = Part.makeBox(12.0, 73.0, 1.5, FreeCAD.Vector(48.0, -73.0, 0.0))
solid_atas = body.fuse([left_wing, right_wing])

cavity = Part.makeBox(33.0, 70.0, 22.5, FreeCAD.Vector(13.5, -71.5, 0.0))
front_bot_cut = Part.makeBox(33.0, 1.5, 4.0, FreeCAD.Vector(13.5, -1.5, 0.0))
front_mid_cut = Part.makeBox(26.0, 1.5, 5.0, FreeCAD.Vector(17.0, -1.5, 4.0))
rear_cut = Part.makeBox(19.0, 1.5, 9.5, FreeCAD.Vector(20.5, -73.0, 13.0))

solid_atas = solid_atas.cut([cavity, front_bot_cut, front_mid_cut, rear_cut])

# 6 Holes for ATAS (Diameter 3.5mm)
holes_atas = []
r_hole = 3.5 / 2.0
h_hole = 10.0
for x in [6.0, 54.0]:
    for y in [-11.5, -36.5, -61.5]:
        cyl = Part.makeCylinder(r_hole, h_hole, FreeCAD.Vector(x, y, -2.0), FreeCAD.Vector(0, 0, 1))
        holes_atas.append(cyl)

solid_atas = solid_atas.cut(holes_atas)
solid_atas = solid_atas.cut(corner_cuts).removeSplitter()
print(f"ATAS Solid Valid: {solid_atas.isValid()}, Volume: {solid_atas.Volume:.2f} mm3")

# Save CASING_NANO_ATAS.FCStd
doc_atas = FreeCAD.newDocument("CASING_NANO_ATAS")
feat_atas = doc_atas.addObject("Part::Feature", "Tutup_Atas")
feat_atas.Shape = solid_atas
feat_atas.Label = "Casing Nano Tutup Atas (Fillet R3 + 6 Lubang M3.5)"
doc_atas.recompute()
doc_atas.saveAs(os.path.join(cwd, "CASING_NANO_ATAS.FCStd"))
FreeCAD.closeDocument("CASING_NANO_ATAS")
print("Saved CASING_NANO_ATAS.FCStd")

# 2. BAWAH (Base 4.0 mm thick + Full-Height Seamless Clearance to Base + 6 Hex Nut Pockets)
base = Part.makeBox(60.0, 73.0, base_thick, FreeCAD.Vector(0.0, -73.0, 0.0))

# Entire raised body has uniform clearance c = 0.30mm all the way down to base_thick (Z = 4.0mm)
body_x_min = 13.5 + c
body_x_max = 46.5 - c
body_dx = body_x_max - body_x_min
body_y_min = -71.5 + c
body_y_max = 0.0
body_dy = body_y_max - body_y_min

raised_body = Part.makeBox(body_dx, body_dy, 4.0, FreeCAD.Vector(body_x_min, body_y_min, base_thick))
solid_bawah = base.fuse(raised_body)

# Internal tray cut (leaving lip of 2.0mm height from Z=6 to 8mm, and floor at Z=6mm)
tray_cut = Part.makeBox(30.0, 70.0, 2.1, FreeCAD.Vector(15.0, -71.5, base_thick + 2.0))
solid_bawah = solid_bawah.cut(tray_cut)

# Internal cutouts through the floor and base
rear_cut_bawah = Part.makeBox(26.0, 42.0, base_thick + 2.1, FreeCAD.Vector(17.0, -67.5, -0.05))
mid_cut_bawah = Part.makeBox(26.0, 12.0, base_thick + 2.1, FreeCAD.Vector(17.0, -17.5, -0.05))
front_shelf = Part.makeBox(26.0, 4.0, 2.1, FreeCAD.Vector(17.0, -5.5, base_thick + 2.0))

solid_bawah = solid_bawah.cut([rear_cut_bawah, mid_cut_bawah, front_shelf])

# 6 Holes and M3 Hex Nut Pockets for BAWAH
holes_bawah = []
nut_traps = []
s_nut = 6.0   # 6.0 mm across flats
h_nut = 2.5   # 2.5 mm pocket depth (nut M3 tebal 2.4 mm tenggelam rapi, sisa dinding atas = 4.0 - 2.5 = 1.5 mm)

for x in [6.0, 54.0]:
    for y in [-11.5, -36.5, -61.5]:
        cyl = Part.makeCylinder(r_hole, base_thick + 4.0, FreeCAD.Vector(x, y, -1.0), FreeCAD.Vector(0, 0, 1))
        holes_bawah.append(cyl)
        nut = make_hex_prism(s_nut, h_nut, x, y, -0.05)
        nut_traps.append(nut)

solid_bawah = solid_bawah.cut(holes_bawah)
solid_bawah = solid_bawah.cut(nut_traps)
solid_bawah = solid_bawah.cut(corner_cuts).removeSplitter()
print(f"BAWAH Solid Valid: {solid_bawah.isValid()}, Volume: {solid_bawah.Volume:.2f} mm3")

# Save CASING_NANO_BAWAH.FCStd
doc_bawah = FreeCAD.newDocument("CASING_NANO_BAWAH")
feat_bawah = doc_bawah.addObject("Part::Feature", "Tutup_Bawah")
feat_bawah.Shape = solid_bawah
feat_bawah.Label = "Casing Nano Tutup Bawah (Tebal 4mm + Nut Trap 2.5mm + Dinding 1.5mm)"
doc_bawah.recompute()
doc_bawah.saveAs(os.path.join(cwd, "CASING_NANO_BAWAH.FCStd"))
FreeCAD.closeDocument("CASING_NANO_BAWAH")
print("Saved CASING_NANO_BAWAH.FCStd")

# 3. ASSEMBLY
doc_assy = FreeCAD.newDocument("CASING_NANO_ASSEMBLY")

feat_bawah_assy = doc_assy.addObject("Part::Feature", "Casing_Bawah")
feat_bawah_assy.Shape = solid_bawah
feat_bawah_assy.Label = "Casing Bawah (Tebal 4mm, Z = 0mm)"

feat_atas_assy = doc_assy.addObject("Part::Feature", "Casing_Atas")
solid_atas_assy = solid_atas.copy()
solid_atas_assy.translate(FreeCAD.Vector(0, 0, base_thick))
feat_atas_assy.Shape = solid_atas_assy
feat_atas_assy.Label = "Casing Atas (Assembled, Z = +4.0mm)"

doc_assy.recompute()
doc_assy.saveAs(os.path.join(cwd, "CASING_NANO_ASSEMBLY.FCStd"))
FreeCAD.closeDocument("CASING_NANO_ASSEMBLY")
print("Saved CASING_NANO_ASSEMBLY.FCStd")

inter = solid_atas_assy.common(solid_bawah)
print(f"Assembly Interference Volume: {inter.Volume:.4f} mm3 (0.0000 = fit sempurna)")
print("ALL FCSTD FILES CREATED SUCCESSFULLY IN FREECAD 1.1.4!")
sys.exit(0)
