// modified from https://github.com/rsheldiii/openSCAD-projects/blob/master/key/src/key.scad

use <rsheldiii-openSCAD-projects/key/src/key.scad>

include <rsheldiii-openSCAD-projects/key/src/settings.scad>
include <rsheldiii-openSCAD-projects/key/src/key_sizes.scad>
include <rsheldiii-openSCAD-projects/key/src/key_profiles.scad>
include <rsheldiii-openSCAD-projects/key/src/key_types.scad>
include <rsheldiii-openSCAD-projects/key/src/key_transformations.scad>

module raise2d(h)
{
    $keytop_thickness = h+.01;
    translate([0,0,h]) intersection()
    {
        linear_extrude(20*h) children();
        keytop();
    }
}

module custom_key()
{
	dsa_row(3) union() { key(); raise2d(1) children(); }
}

custom_key() translate([-9,-9]) import(glyph);
