// modified from https://github.com/rsheldiii/openSCAD-projects/blob/master/key/src/key.scad

use <rsheldiii-openSCAD-projects/key/src/key.scad>

include <rsheldiii-openSCAD-projects/key/src/settings.scad>
include <rsheldiii-openSCAD-projects/key/src/key_sizes.scad>
include <rsheldiii-openSCAD-projects/key/src/key_profiles.scad>
include <rsheldiii-openSCAD-projects/key/src/key_types.scad>
include <rsheldiii-openSCAD-projects/key/src/key_transformations.scad>


module fdm()
{
    intersection()
    {
    translate([-10,-10,0])cube([20,20,6.9]);
    children();
    }
}

module raise2d(h)
{
    $keytop_thickness = h+.01;
    translate([0,0,h]) intersection()
    {
        linear_extrude(20*h) children();
        fdm() translate([-10,-10,5]) cube([20,20,10]);
    }
}

module custom_key()
{
	dsa_row(3) union() { fdm() key(); raise2d(1) children(); }
}

//glyph="svg/gb.svg";
$keytop_thickness = 2;
$wall_thickness = 4;

custom_key() translate([-9,-9]) import(glyph);
