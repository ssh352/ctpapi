{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
  {
    'target_name' : 'rong_hang_trader',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
# '<!@(python ../build/glob_files.py . *.h *.cc)',
      'main.cc',
      'rohon_trade_api.h',
      'rohon_trade_api.cc',
    ],
    'dependencies' : [
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
# '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
      '<(DEPTH)/third_party/rohon/rohon.gyp:*',
      '../hpt_core/hpt_core.gyp:hpt_core',
# '../caf_ctp/caf_ctp.gyp:*',
    ],
    'defines' : [
# 'H5_BUILT_AS_DYNAMIC_LIB',
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
    ],

  },
  {
    'target_name' : 'test_remote_actor',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
#'<!@(python ../build/glob_files.py . *.h *.cc)',
      'test_remote_actor.cc',
    ],
    'dependencies' : [
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
    ],
    'defines' : [
# 'H5_BUILT_AS_DYNAMIC_LIB',
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
    ],

  }
  ]
}
