{
  'targets' : [
    {
      'target_name' : 'geek_quant_demo',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        'main.cc',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
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
        'follow_strategy.h',
        'follow_strategy.cc',
        'caf_defines.h',
        'ctp_trader.h',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
    },
    {
      'target_name' : 'strategy_unittest',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        'strategy_unittest.cc',
      ],
      'dependencies' : [
        'libgeekquant',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtest_main',
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
    }
  ]
}     
