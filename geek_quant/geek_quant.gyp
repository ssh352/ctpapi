{
  'targets' : [
    {
      'target_name' : 'geek_quant_demo',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        'main.cc',
        'follow_strategy.h',
        'caf_defines.h',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        '<(DEPTH)/third_party/tradeapi/tradeapi.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs': [
        '..',
      ],
    },
    {
      'target_name' : 'libgeekquant',
      'type' : '<(component)',
      'variables' : {
      },
      'sources' : [
      ],
      'dependencies' : [
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
      ],
    },
  ]
}     
