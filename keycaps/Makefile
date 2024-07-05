
SVGS=$(wildcard svg/*.svg)
SLA_STLS=$(patsubst svg/%.svg, stl/%_sla.stl, $(SVGS))
FDM_STLS=$(patsubst svg/%.svg, stl/%_fdm.stl, $(SVGS))

TARGETS=$(FDM_STLS) $(SLA_STLS)

all: $(TARGETS)

stl/%_sla.stl: svg/%.svg sla_keys.scad
	openscad sla_keys.scad -D 'glyph="'"$<"'"' -o $@

stl/%_fdm.stl: svg/%.svg fdm_keys.scad
	openscad fdm_keys.scad -D 'glyph="'"$<"'"' -o $@

