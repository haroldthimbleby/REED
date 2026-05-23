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
            
            $record = "<tr><td>$date</td><td>$author</td><td>$email</td><td>$version</td><td>$compiled</td><td style=\"color:firebrick\">$feedback</td></tr>\n";
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

			echo "<h1>REED version ". htmlspecialchars($version)." feedback given ". htmlspecialchars($date)."</h1><p style=\"color:firebrick\">". htmlspecialchars($feedback)."<p/>&mdash; from: ". htmlspecialchars($author)."<br/>&mdash; ". htmlspecialchars($email);
            echo "<p/><hr/>Thank you for supporting REEDs! Do email <a href=\"mailto:harold@thimbleby.net\"><tt>harold@thimbleby.net</tt></a> with any ideas or criticisms.<p/><hr/><p/>";
            
            $sent = mail('harold@thimbleby.net', 'REED feedback', "$date\n$author\n$email\n$feedback\n");
            
            echo "Sent email $sent ...</br>";
        ?>
    </body>
</html>
