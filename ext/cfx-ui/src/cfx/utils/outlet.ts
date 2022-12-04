import React from 'react';
import ReactDOM from 'react-dom';

export function attachOutlet(id: string): ChildrenfulReactFC {
  const outlet = document.getElementById(id);

  if (!outlet) {
    throw new Error(`Tried to attach outlet to the DOM node #${id}, but there is no such`);
  }

  return ({ children }) => ReactDOM.createPortal(children, outlet);
}

export function createOutlet(): ChildrenfulReactFC {
  const outlet = document.createElement('div');

  document.body.appendChild(outlet);

  return ({ children }) => ReactDOM.createPortal(children, outlet);
}
