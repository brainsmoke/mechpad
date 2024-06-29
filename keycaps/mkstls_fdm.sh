#!/bin/bash

mkdir -p stl

for svg in svg/g?.svg; do

	base="${svg#svg/}"
	stlname="stl/${base%.svg}_fdm.stl"
	openscad fdm_keys.scad -D 'glyph="'"$svg"'"' -o "$stlname"

done
