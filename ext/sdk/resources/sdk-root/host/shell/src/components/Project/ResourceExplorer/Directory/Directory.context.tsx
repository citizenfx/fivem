import React from 'react';
import { FilesystemEntry } from '../../../../sdkApi/api.types';
import { defaultVisibilityFilter } from '../../../Explorer/Explorer';

export interface DirectoryContext {
  visibilityFilter: (entry: FilesystemEntry) => boolean,
  forbidDeleteDirectory: boolean,
  forbidCreateDirectory: boolean,
  forbidCreateResource: boolean,
}
export const DirectoryContext = React.createContext({
  visibilityFilter: defaultVisibilityFilter,
  forbidDeleteDirectory: false,
  forbidCreateDirectory: false,
  forbidCreateResource: false,
});

export const DirectoryContextProdiver = React.memo((props: Partial<DirectoryContext> & { children: React.ReactNode }) => {
  const defaults = React.useContext(DirectoryContext);
  const { children, ...contextOverrides } = props;

  const value = {
    ...defaults,
    ...contextOverrides,
  };

  return (
    <DirectoryContext.Provider value={value}>
      {children}
    </DirectoryContext.Provider>
  );
});
