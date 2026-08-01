<?php
$ffi = FFI::cdef(
    "int MovieMakerMain(int argc, char **argv);",
    "../src/MovieMakerCore/Debug/MovieMakerCore.dll"
);
$res = $ffi->MovieMakerMain(0, NULL);
var_dump($res);
?>
