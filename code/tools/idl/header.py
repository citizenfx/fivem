# add dependency directory to the Python path
import site
import os.path

site.addsitedir(os.path.join(os.path.dirname(__file__), 'deps'))

# call header.py
from xpidl import header

header.main()