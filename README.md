maurina
=======

Maurina is a server side languages console. It was designed to help debug programs that execute on a server by displaying error messages, debug messages, info about sessions, HTTP requests, etc...

It was designed with PHP in mind, because it's the server side language I use, but it can be used with any other programming language since it's just an UDP server that can receive data from any program.

It is written in Qt/C++ and it's free as in free beer and free speech.

You can find detailed info and executable download links at our website [maurina.org](http://maurina.org).

As a teaser you can see below some screenshots and the code that produced them:

![Alt text](http://www.maurina.org/images/cap1_en.png)
![Alt text](http://www.maurina.org/images/cap2_en.png)
![Alt text](http://www.maurina.org/images/cap3_en.png)
![Alt text](http://www.maurina.org/images/cap4_en.png)

    // Start session and set some values
    session_start();
    $_SESSION['USER_ID'] = 25;
    $_SESSION['USER_NAME'] = 'User name';

    // Set some REQUEST (POST or GET) values
    $_REQUEST['param1'] = 'value1';

    // Create Maurina instance
    include('Maurina.php');
    $M = new Maurina();

    // Send some custom messages
    $M->log('Message sent using the log() method.');
    $M->log('Another message.');

    // Some code that will raise errors
    $a = $a / 0;
