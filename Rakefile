ENV['MRUBY_CONFIG'] = File.join(File.dirname(__FILE__), '.', 'build_config.rb')

unless File.exists?('submodules/mruby/Rakefile')
  raise "mruby's Rakefile not found: try 'git submodule init' or 'git submodule update' first!"
end

load 'submodules/mruby/Rakefile'

LIBMRUBY = File.expand_path('submodules/mruby/build/arm-cortex-m4/lib/libmruby.a')

desc 'build libmruby'
task :libmruby => LIBMRUBY
