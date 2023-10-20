#!/bin/bash

mkdir -p stl

for svg in svg/g?.svg; do

	base="${svg#svg/}"
	stlname="stl/${base%.svg}.stl"
	openscad flat_keys.scad -D 'glyph="'"$svg"'"' -o "$stlname"

done
