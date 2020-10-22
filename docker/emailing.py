import os
import smtplib
import zipfile
import tempfile

from email import encoders
from email.message import Message
from email.mime.base import MIMEBase
from email.mime.multipart import MIMEMultipart  

def send_email(the_files, *, recipients, subject, sender='ian.bell@nist.gov'):
    """
    https://stackoverflow.com/questions/10474650/how-to-send-a-zip-file-as-an-attachment-in-python
    """

    # Create the message
    themsg = MIMEMultipart()
    themsg['Subject'] = subject
    to = ', '.join(recipients)
    themsg['To'] = to
    themsg['From'] = sender
    themsg['Contents'] = "something"
    themsg.preamble = 'I am not using a MIME-aware mail reader.\n'

    for the_file in the_files:
        if the_file is not None:
            ext = os.path.split(the_file)[1].split('.')[1]
            if ext == 'zip':
                msg = MIMEBase('application', 'zip')
                with open(the_file, 'rb') as zf:
                    msg.set_payload(zf.read())
                encoders.encode_base64(msg)
            elif ext == 'html':
                msg = MIMEBase('text', 'html')
                with open(the_file) as fp:
                    msg.set_payload(fp.read())
            else:
                raise ValueError("Don't know how to attach this file:" + the_file)
            msg.add_header('Content-Disposition', 'attachment', 
                           filename=os.path.basename(the_file))
            themsg.attach(msg)

    # Send the email via SMTP
    with smtplib.SMTP('smtp.nist.gov') as smtp:
        smtp.ehlo(); # print(smtp.ehlo_resp)
        smtp.send_message(themsg)

if __name__=='__main__':
    files = ['test_build_run.html', 'test.zip', 'gcov_build_run.html', 'gcov.zip']
    send_email(files, recipients=['ian.bell@nist.gov'], subject='If you get this email, please email me')