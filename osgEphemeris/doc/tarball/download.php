 <h3>Download</hr>

 <p>
 <h4> Release Tarballs </h4>
 The following releases are available for download:
 <blockquote>
 <?
    function getDirectoryListing( $dir )
    {
        $a = array();

        if ($handle = opendir($dir))
        {
            $index = 0;
            while (false !== ($file = readdir($handle)))
            {
                if( $file != "." && $file != ".." && $file != "CVS")
                {
                    $a[$index] = $dir . "/" . $file;
                    $index ++;
                }
            }
            closedir( $handle );
            return $a;
        }
        return NULL;
    }


    $d = getDirectoryListing( "Download/Releases" );

    rsort($d);

    foreach( $d as $f )
    { ?>
        <a href="<?=$f?>"><?=basename($f)?></a><br>
  <?}

 ?>
 </blockquote>

 <h4> CVS access </h4>
 <p>
 For those wishing to get the latest copy in CVS, check out from the command line
 as follows:
 <pre> <code> <font size=+1>
    cvs -d :pserver:cvsguest@andesengineering.com:/cvs/Producer co Producer
 </font></code></pre>

 <h4> Generated Tarballs </h4>
 <p>
 If you can't, or you prefer not to use CVS, you may get a current snapshot here:
 <p>
 <? include "tarball.php" ?>

