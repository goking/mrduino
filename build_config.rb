MRuby::Build.new do |conf|
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  enable_debug

  # include the default GEMs
  conf.gembox 'default'
end

MRuby::Toolchain.new(:arm_none_eabi_gcc) do |conf|
  prefix = "arm-none-eabi-"
  toolchain :gcc
  [conf.cc, conf.linker].each do |cc|
    cc.command = prefix + (ENV['CC'] || 'gcc')
  end
  conf.archiver.command = prefix + (ENV['AR'] || 'ar')
end

MRuby::CrossBuild.new('arm-cortex-m4') do |conf|
  toolchain :arm_none_eabi_gcc

  conf.cc.flags << %w(-g -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16)

  #macros = %w(MRB_HEAP_PAGE_SIZE=256 GC_STEP_SIZE=256)
  #conf.cc.flags << macros.map {|m| "-D\"#{m}\""}

  conf.bins = []
end
