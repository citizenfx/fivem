import React, { HTMLAttributes } from 'react';
import classnames from 'classnames';

type Stylesheet = Record<string, string>;

export type StyledProps<DefinedClassNames extends Record<string, boolean>, Element> = DefinedClassNames & {
  props?: React.DetailedHTMLProps<HTMLAttributes<Element>, Element>,
  children?: React.ReactNode,
}

export const div = <DefinedClassNames extends Record<string, boolean> = {}>(stylesheets: Stylesheet | Stylesheet[], defaultClassName: string = '') => {
  const stylesheet = Array.isArray(stylesheets)
    ? stylesheets.reduce((acc, ss) => ({ ...acc, ...ss }), {})
    : stylesheets;

  const classNameComposer = (props: StyledProps<DefinedClassNames, HTMLDivElement>): string => classnames(
    stylesheet[defaultClassName],
    Object.keys(props).reduce((acc, className) => {
      if (className === 'props' || className === 'children') {
        return acc;
      }

      acc[stylesheet[className]] = props[className];

      return acc;
    }, {}),
  );

  return (props: StyledProps<DefinedClassNames, HTMLDivElement>) => {
    const className = classNameComposer(props);

    const divProps: typeof props.props = {
      ...(props.props || {}),
      className,
    };

    return (
      <div {...divProps}>
        {props.children}
      </div>
    );
  };
};
