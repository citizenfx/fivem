import React from 'react';

export function renderedIf<Props extends JSX.IntrinsicAttributes = object>(
  precondition: () => boolean,
  Component: React.FC<Props>,
) {
  return function RenderedIf(props: Props) {
    if (!precondition()) {
      return null;
    }

    return (
      <Component {...props} />
    );
  };
}
