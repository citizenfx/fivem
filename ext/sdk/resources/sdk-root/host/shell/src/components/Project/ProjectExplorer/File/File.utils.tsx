import * as React from 'react';
import { BiFile } from 'react-icons/bi';
import { BsCardChecklist } from 'react-icons/bs';
import { SiJavascript, SiJson, SiLua } from 'react-icons/si';
import { FilesystemEntry } from 'sdkApi/api.types';

export const getFileIcon = (entry: FilesystemEntry): React.ReactNode => {
  if (entry.name === '__resource.lua' || entry.name === 'fxmanifest.lua') {
    return <BsCardChecklist />;
  }

  const indexOfExtDot = entry.name.lastIndexOf('.');
  let ext = 'generic';

  if (indexOfExtDot) {
    ext = entry.name.substr(indexOfExtDot + 1);
  }

  switch (ext) {
    case 'js': return <SiJavascript />;
    case 'lua': return <SiLua />;
    case 'json': return <SiJson />;

    case 'generic':
    default: {
      return <BiFile />;
    }
  }
};
