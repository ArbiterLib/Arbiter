#!/usr/bin/env ruby

require 'mkmf'

$LDFLAGS << ' -lc++'

find_header('arbiter/Dependency.h', "../../include")
find_library('Arbiter', 'ArbiterCreateProjectIdentifier', "../..")

create_makefile('arbiter')
