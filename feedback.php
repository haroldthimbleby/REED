<!-- 
	feedback.php?feedback=xyz&author=abc&date=22-May-2026
  -->
<html>
    <body>
        <?php
        	$feedbackFileName = "feedback.txt";
        
            $feedback = urldecode($_GET['feedback']);
            $author = urldecode($_GET['author']);
            $date = urldecode($_GET['date']);            
            $email = urldecode($_GET['email']);
            $version = urldecode($_GET['version']);
            $compiled = urldecode($_GET['compiled']);
            
            $record = "$date\t$author\t$email\t$version\t$compiled\t$feedback\n****\n";
            $email = $email != ""? "email: $email": "[no email supplied]";
            
            $feedbackfd = fopen($feedbackFileName, "a+"); 
            
            if( $feedbackfd ) // don't cause problems if we can't open the file
            {   if( flock($feedbackfd, LOCK_EX) )
                {  //echo "Locked $feedbackFileName<br/>";
					fwrite($feedbackfd, $record);
                   //echo "Wrote to $feedbackFileName<br/>";
					flock($feedbackfd, LOCK_UN);
				   //echo "Closed $feedbackFileName<br/>";
                }
                else echo "Oops! Error locking $feedbackFileName";
                fclose($feedbackfd);
            }

			echo "<h1>REED version ". htmlspecialchars($version)." feedback given ". htmlspecialchars($date)."</h1>". htmlspecialchars($feedback)."<p/>&mdash; from: ". htmlspecialchars($author)."<br/>&mdash; ". htmlspecialchars($email);
            echo "<p/><hr/>Thank you for supporting REED. Do email <tt>harold@thimbleby.net</tt> with any criticisms or ideas.<p/><hr/><p/>";
        ?>
    </body>
</html>
