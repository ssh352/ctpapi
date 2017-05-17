{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'download_margin_rate',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py . *.h *.cc)',
		'../follow_trade_server/ctp_trader.h',
		'../follow_trade_server/ctp_trader.cc',
        '../follow_trade_server/util.h',
        '../follow_trade_server/util.cc',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
        '<(DEPTH)/follow_strategy_mode/follow_strategy_mode.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
    },
  ]
}
