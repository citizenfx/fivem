import React from 'react';
import ReactDOM from 'react-dom';

export function attachOutlet(id: string): React.FC {
  const outlet = document.getElementById(id);

  return ({ children }) => ReactDOM.createPortal(children, outlet);
}

export function createOutlet(): React.FC {
  const outlet = document.createElement('div');

  document.body.appendChild(outlet);

  return ({ children }) => ReactDOM.createPortal(children, outlet);
}
