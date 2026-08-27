(module
  (import "occt" "memory" (memory 256 32768))
  (import "occt" "k_alloc" (func $alloc (param i32) (result i32)))
  (import "occt" "k_free" (func $free (param i32)))
  (import "occt" "k_handle" (func $handle (param i32 i32) (result i32)))
  (import "occt" "k_response_ptr" (func $response_ptr (result i32)))
  (import "occt" "k_buffer_ptr" (func (param i32) (result i32)))
  (import "occt" "k_buffer_len" (func (param i32) (result i32)))

  (data $begin_scope_request "{\22id\22:1,\22op\22:\22beginScope\22,\22args\22:{}}")
  (data $make_box_request "{\22id\22:2,\22op\22:\22makeBox\22,\22args\22:{\22scopeId\22:1,\22size\22:[2,3,4]}}")
  (data $make_cylinder_request "{\22id\22:3,\22op\22:\22makeCylinder\22,\22args\22:{\22scopeId\22:1,\22radius\22:0.5,\22height\22:4,\22origin\22:[1,1.5,0]}}")
  (data $boolean_cut_request "{\22id\22:4,\22op\22:\22booleanCut\22,\22args\22:{\22scopeId\22:1,\22base\22:1048577,\22tools\22:[1048578]}}")
  (data $bbox_request "{\22id\22:5,\22op\22:\22bbox\22,\22args\22:{\22shape\22:1048579}}")
  (data $tessellate_request "{\22id\22:6,\22op\22:\22tessellate\22,\22args\22:{\22shape\22:1048579,\22linearDeflection\22:0.1,\22angularDeflection\22:0.5}}")
  (data $invalid_box_request "{\22id\22:7,\22op\22:\22makeBox\22,\22args\22:{\22scopeId\22:1,\22size\22:[0,1,1]}}")
  (data $release_all_request "{\22id\22:8,\22op\22:\22releaseAll\22,\22args\22:{}}")
  (data $stats_request "{\22id\22:9,\22op\22:\22stats\22,\22args\22:{}}")
  (data $response_prefix "{\22id\22:")
  (data $success_suffix ",\22ok\22:true")
  (data $invalid_args_prefix "{\22error\22:{\22code\22:\22InvalidArgs\22")
  (data $stats_response_prefix "{\22id\22:9,\22ok\22:true,\22result\22:{\22liveBufferBytes\22:0,\22liveShapeHandles\22:0,")

  (func $assert_equal (param $actual i32) (param $expected i32) (param $length i32)
    (local $index i32)
    (block $done
      (loop $next
        (br_if $done (i32.eq (local.get $index) (local.get $length)))
        (if (i32.ne
          (i32.load8_u (i32.add (local.get $actual) (local.get $index)))
          (i32.load8_u (i32.add (local.get $expected) (local.get $index))))
          (then unreachable))
        (local.set $index (i32.add (local.get $index) (i32.const 1)))
        (br $next))))

  (func $assert_response (param $length i32) (param $id i32) (param $success i32)
    (local $actual i32)
    (local $expected i32)
    (if (i32.lt_u (local.get $length) (i32.const 18)) (then unreachable))
    (local.set $actual (call $response_ptr))
    (if (local.get $success)
      (then
        (local.set $expected (call $alloc (i32.const 6)))
        (memory.init $response_prefix (local.get $expected) (i32.const 0) (i32.const 6))
        (call $assert_equal (local.get $actual) (local.get $expected) (i32.const 6))
        (call $free (local.get $expected))
        (if (i32.ne
          (i32.load8_u (i32.add (local.get $actual) (i32.const 6)))
          (i32.add (local.get $id) (i32.const 48)))
          (then unreachable))
        (local.set $expected (call $alloc (i32.const 10)))
        (memory.init $success_suffix (local.get $expected) (i32.const 0) (i32.const 10))
        (call $assert_equal (i32.add (local.get $actual) (i32.const 7)) (local.get $expected) (i32.const 10))
        (call $free (local.get $expected)))
      (else
        (if (i32.lt_u (local.get $length) (i32.const 30)) (then unreachable))
        (local.set $expected (call $alloc (i32.const 30)))
        (memory.init $invalid_args_prefix (local.get $expected) (i32.const 0) (i32.const 30))
        (call $assert_equal (local.get $actual) (local.get $expected) (i32.const 30))
        (call $free (local.get $expected)))))

  (func $begin_scope
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 36)))
    (memory.init $begin_scope_request (local.get $request) (i32.const 0) (i32.const 36))
    (local.set $length (call $handle (local.get $request) (i32.const 36)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 1) (i32.const 1)))

  (func $make_box
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 59)))
    (memory.init $make_box_request (local.get $request) (i32.const 0) (i32.const 59))
    (local.set $length (call $handle (local.get $request) (i32.const 59)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 2) (i32.const 1)))

  (func $make_cylinder
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 92)))
    (memory.init $make_cylinder_request (local.get $request) (i32.const 0) (i32.const 92))
    (local.set $length (call $handle (local.get $request) (i32.const 92)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 3) (i32.const 1)))

  (func $boolean_cut
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 80)))
    (memory.init $boolean_cut_request (local.get $request) (i32.const 0) (i32.const 80))
    (local.set $length (call $handle (local.get $request) (i32.const 80)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 4) (i32.const 1)))

  (func $bbox
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 45)))
    (memory.init $bbox_request (local.get $request) (i32.const 0) (i32.const 45))
    (local.set $length (call $handle (local.get $request) (i32.const 45)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 5) (i32.const 1)))

  (func $tessellate
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 98)))
    (memory.init $tessellate_request (local.get $request) (i32.const 0) (i32.const 98))
    (local.set $length (call $handle (local.get $request) (i32.const 98)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 6) (i32.const 1)))

  (func $invalid_box
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 59)))
    (memory.init $invalid_box_request (local.get $request) (i32.const 0) (i32.const 59))
    (local.set $length (call $handle (local.get $request) (i32.const 59)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 7) (i32.const 0)))

  (func $release_all
    (local $request i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 36)))
    (memory.init $release_all_request (local.get $request) (i32.const 0) (i32.const 36))
    (local.set $length (call $handle (local.get $request) (i32.const 36)))
    (call $free (local.get $request))
    (call $assert_response (local.get $length) (i32.const 8) (i32.const 1)))

  (func $stats
    (local $request i32) (local $actual i32) (local $expected i32) (local $length i32)
    (local.set $request (call $alloc (i32.const 31)))
    (memory.init $stats_request (local.get $request) (i32.const 0) (i32.const 31))
    (local.set $length (call $handle (local.get $request) (i32.const 31)))
    (call $free (local.get $request))
    (if (i32.lt_u (local.get $length) (i32.const 69)) (then unreachable))
    (local.set $actual (call $response_ptr))
    (local.set $expected (call $alloc (i32.const 69)))
    (memory.init $stats_response_prefix (local.get $expected) (i32.const 0) (i32.const 69))
    (call $assert_equal (local.get $actual) (local.get $expected) (i32.const 69))
    (call $free (local.get $expected)))

  (func (export "_start")
    (call $begin_scope)
    (call $make_box)
    (call $make_cylinder)
    (call $boolean_cut)
    (call $bbox)
    (call $tessellate)
    (call $invalid_box)
    (call $release_all)
    (call $stats)))
