ENV['MRUBY_CONFIG'] = File.join(File.dirname(__FILE__), '.', 'build_config.rb')

unless File.exists?('submodules/mruby/Rakefile')
  raise "mruby's Rakefile not found: try 'git submodule init' or 'git submodule update' first!"
end

load 'submodules/mruby/Rakefile'

MRB_BUILD = File.expand_path('submodules/mruby/build')

MRBC = File.expand_path('host/bin/mrbc', MRB_BUILD)
LIBMRUBY = File.expand_path('arm-cortex-m4/lib/libmruby.a', MRB_BUILD)

desc 'build libmruby'
task :libmruby => LIBMRUBY

desc 'build mrbc'
task :mrbc => MRBC
