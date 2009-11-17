<?  
    //$product=$_POST['product'];
    //$repository=$_POST['repository'];

    $ret  = system( "maketarball" );

    if( $ret != NULL )
        echo $ret . "<br> \n";
    else
    {
?>

<html>
<META http-equiv="Refresh" content="0;url=index.php">
</html>

<?    }  ?>
