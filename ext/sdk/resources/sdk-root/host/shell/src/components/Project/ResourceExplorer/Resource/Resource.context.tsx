import React from 'react';

export interface ResourceContext {
  forbidDeletion: boolean,
  forbidRenaming: boolean,
}
export const ResourceContext = React.createContext<ResourceContext>({
  forbidDeletion: false,
  forbidRenaming: false,
});

export const ResourceContextProvider = React.memo((props: Partial<ResourceContext> & { children: React.ReactNode }) => {
  const defaults = React.useContext(ResourceContext);
  const { children, ...contextOverrides } = props;

  const value = {
    ...defaults,
    ...contextOverrides,
  };

  return (
    <ResourceContext.Provider value={value}>
      {children}
    </ResourceContext.Provider>
  );
});
