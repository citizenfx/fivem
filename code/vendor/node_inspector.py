import os
import subprocess
import sys

noderoot = sys.argv[1]

try:
    os.makedirs(os.path.join(noderoot, 'gen/src/node/inspector/protocol/'))
except:
    pass

subprocess.check_call(['python', 'tools/inspector_protocol/convert_protocol_to_json.py', 'src/inspector/node_protocol.pdl', 'gen/src/node_protocol.json'], cwd = noderoot)
subprocess.check_call(['python', 'tools/inspector_protocol/code_generator.py', '--jinja_dir', 'tools/inspector_protocol','--output_base', 'gen/src/', '--config', 'src/inspector/node_protocol_config.json'], cwd = noderoot)
subprocess.check_call(['python', 'tools/inspector_protocol/concatenate_protocols.py', 'deps/v8/include/js_protocol.pdl', 'gen/src/node_protocol.json', 'gen/src/node/inspector/protocol/concatenated_protocol.json'], cwd = noderoot)
subprocess.check_call(['python', 'tools/compress_json.py', 'gen/src/node/inspector/protocol/concatenated_protocol.json', 'gen/src/node/inspector/protocol/v8_inspector_protocol_json.h'], cwd = noderoot)