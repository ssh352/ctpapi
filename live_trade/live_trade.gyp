{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'live_trade',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py . *.h *.cc *.cpp)',
      ],
      'sources!' : [
        'backroll.cc',
      ],
      'sources/' :[
        ['exclude', 'backroll.cc'],
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
        '<(DEPTH)/third_party/sqlite/sqlite.gyp:*',
        '<(DEPTH)/third_party/yaml-cpp/yaml-cpp.gyp:*',
        '../hpt_core/hpt_core.gyp:hpt_core',
        '../bft_core/bft_core.gyp:*',
        '../follow_strategy/follow_strategy.gyp:follow_strategy',
        '../ctp_broker/ctp_broker.gyp:ctp_broker',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],

      },
      # {
      # 'target_name' : 'backroll',
      # 'type' : 'executable',
      # 'variables' : {
      # },
      # 'sources' : [
        # 'backroll.cc',
      # ],
      # 'dependencies' : [
        # '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        # '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
        # '../hpt_core/hpt_core.gyp:hpt_core',
        # '../strategies/strategies.gyp:strategies',
        # '../follow_strategy/follow_strategy.gyp:follow_strategy',
        # '../ctp_broker/ctp_broker.gyp:ctp_broker',
      # ],
      # 'defines' : [
      # ],
      # 'includes' : [
      # ],
      # 'include_dirs' : [
        # '..',
      # ],

      # }
  ]
}
