<!DOCTYPE html>
<!-- 
	feedback.php?feedback=xyz&author=abc&date=22-May-2026&email...
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
            
            if( $email == "" ) $email = "[no email supplied]";
            
            // the $actualFeedback is used to search the feedback.txt file to see
            // if it has been previously sent. The HTML and other junk in $actualFeedback is
            // intended to avoid accidentally matching substrings of the feedback
            // when we check for duplication of feedback below
            $actualFeedback = "<p style=\"color:firebrick\">".htmlspecialchars($feedback)."</p>";

            $record = "<div style=\"font-family: sans-serif;\">".
            	"<table>\n".
            	"<tr><td colspan=2><b>REED v".htmlspecialchars($version)." compiled ".htmlspecialchars($compiled).
            	"</b></td></tr>\n".
				"<tr><td><b>REED file date</b>:</td><td>".htmlspecialchars($date).
            	".</td></tr><tr><td><b>From</b>:</td><td>".htmlspecialchars($author).
            	"</td></tr><tr><td><b>Reply to</b>:</td><td><a href=\"mailto:".htmlspecialchars($email)."\">".
				"<tt>".htmlspecialchars($email)."</tt></a></td></tr></table></div><p><blockquote>".
				$actualFeedback."\n</blockquote>\n<hr/>";

 			$feedbackstr = file_get_contents($feedbackFileName); // have we recorded this feedback before, if so quit...
			if( $feedbackstr != false )
			{	if( str_contains($feedbackstr, $actualFeedback) )
				{	echo "<span  style=\"font-family: sans-serif;\"><b>Previously saved feedback will not be sent again</b>.</span><p/><hr/>".$actualFeedback."<hr/>\n";
					exit;
				}
			}

            $feedbackfd = fopen($feedbackFileName, "a+"); 
            
            if( $feedbackfd ) // don't cause problems if we can't open the file
            {   if( flock($feedbackfd, LOCK_EX) )
                {  	//echo "Locked $feedbackFileName<br/>";
					fwrite($feedbackfd, $record);
                   	//echo "Wrote to $feedbackFileName<br/>";
					flock($feedbackfd, LOCK_UN);
				   	//echo "Closed $feedbackFileName<br/>";
                }
                else echo "Oops! Error locking $feedbackFileName";
                fclose($feedbackfd);
            }

			echo "<h1>REED feedback</h1>\n".$record;
            echo "Thank you for supporting REEDs! Do email <a href=\"mailto:harold@thimbleby.net\"><tt>harold@thimbleby.net</tt></a> with any ideas or criticisms.<hr/><p/>";
            
           // $sent = mail('harold@thimbleby.net', 'REED feedback', "$date\n$author\n$email\n$feedback\n");
           // echo "Sent email $sent ...</br>";
        ?>
    </body>
</html>
