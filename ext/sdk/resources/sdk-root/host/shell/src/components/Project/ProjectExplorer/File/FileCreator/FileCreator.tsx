import React, { ChangeEvent } from 'react';
import { newFileIcon } from '../../../../../constants/icons';
import { fileNamePattern } from '../../../../../constants/patterns';


export interface FileCreatorProps {
  onCreate: (name: string) => void,
  className: string,
}

export const FileCreator = React.memo(function FileCreator({ onCreate, className }: FileCreatorProps) {
  const [name, setName] = React.useState('');

  const handleChange = React.useCallback((event: ChangeEvent) => {
    const { value } = event.target as any;

    if (fileNamePattern.test(value) || !value) {
      setName(value);
    }
  }, [setName]);

  const handleFocusLost = React.useCallback(() => {
    onCreate(name);
  }, [name, onCreate]);

  const handleKeyDown = React.useCallback((event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.key === 'Enter') {
      event.stopPropagation();
      return onCreate(name);
    }

    if (event.key === 'Escape') {
      event.stopPropagation();
      return onCreate('');
    }
  }, [name, onCreate]);

  return (
    <div className={className}>
      {newFileIcon}
      <input
        autoFocus
        type="text"
        value={name}
        placeholder="File name"
        onChange={handleChange}
        onBlur={handleFocusLost}
        onKeyDown={handleKeyDown}
      />
    </div>
  );
});
