# Import smtplib for the actual sending function
import smtplib
import sys

# Import the email modules we'll need
from email.mime.text import MIMEText

# Open a plain text file for reading.  For this example, assume that
# the text file contains only ASCII characters.
def sendemail(title, text, to):
    # Create a text/plain message
    msg = MIMEText(text)

    username = 'wcautotest@gmail.com'
    password = 'pythontester'

    msg['Subject'] = title
    msg['From'] = username
    msg['To'] = to

    # Send the message via our own SMTP server, but don't include the
    # envelope header.
    server = smtplib.SMTP('smtp.gmail.com:587')
    server.starttls()
    server.login(username,password)
    server.sendmail(username, to, msg.as_string())
    server.quit()

if(len(sys.argv) == 4):
    fp = open(sys.argv[1], 'rb')
    title = fp.read()
    fp.close()

    fp = open(sys.argv[2], 'rb')
    contents = fp.read()
    fp.close()

    recipient = sys.argv[3]

    sendemail(title, contents, recipient)
