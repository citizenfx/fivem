import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import s from './Interactive.module.scss';

export interface InteractiveProps
  extends React.DetailedHTMLProps<React.HTMLAttributes<HTMLDivElement>, HTMLDivElement> {
  showPointer?: boolean;
}

export const Interactive = React.forwardRef(function Interactive(
  props: InteractiveProps,
  ref: React.Ref<HTMLDivElement>,
) {
  const {
    showPointer = true,
    className,
    ...restProps
  } = props;
  const finalClassName = clsx(className, {
    [s.root]: showPointer,
  });

  return (
    <div ref={ref} className={finalClassName} {...restProps}>
      {props.children}
    </div>
  );
});
