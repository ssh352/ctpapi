{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'ctp_bind',
      'type' : '<(component)',
      'variables' : {
      },
      'sources' : [
       '<!@(python ../build/glob_files.py . *.h *.cc *.cpp)',
      ],
      'sources!':[
        'main.cc'
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
        '<(DEPTH)/third_party/actor-framework/libcaf_core/libcaf_core.gyp:*',
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
      'target_name' : 'ctp_bind_demo',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        'main.cc'
      ],
      'dependencies' : [
        'ctp_bind',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
      ],
      'defines' : [
        'BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS',
        'BOOST_MPL_LIMIT_LIST_SIZE=50',
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
    }
  ]
}
