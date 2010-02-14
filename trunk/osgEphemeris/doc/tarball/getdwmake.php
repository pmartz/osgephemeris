<?PHP

    //$VERSION = "1.0-"  . date("YmdHi" );
    $NAME    = "DWMake";
    $TARBALL = $NAME . ".tgz";

     $ret = system( "./make_dwmake_tarball.sh $NAME" );
     $filesize = filesize( $TARBALL );



     header("X-Sendfile: $TARBALL" );
     header("Content-Disposition: attachment; filename=$TARBALL");
     header("Content-Type: application/x-gzip");
     header("Content-Length: $filesize" );

    //header("Content-Transfer-Encoding: binary");

     readfile( $TARBALL );

     unlink( $TARBALL );
?>

