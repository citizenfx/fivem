import React, { ChangeEvent } from 'react';
import { newDirectoryIcon } from '../../../../../constants/icons';
import { directoryNamePattern } from '../../../../../constants/patterns';


export interface DirectoryCreatorProps {
  onCreate: (name: string) => void,
  className: string,
}

export const DirectoryCreator = React.memo(({ onCreate, className }: DirectoryCreatorProps) => {
  const [name, setName] = React.useState('');

  const handleChange = React.useCallback((event: ChangeEvent) => {
    const { value } = event.target as any;

    if (directoryNamePattern.test(value) || !value) {
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
      {newDirectoryIcon}
      <input
        autoFocus
        type="text"
        value={name}
        placeholder="Directory name"
        onChange={handleChange}
        onBlur={handleFocusLost}
        onKeyDown={handleKeyDown}
      />
    </div>
  );
});
