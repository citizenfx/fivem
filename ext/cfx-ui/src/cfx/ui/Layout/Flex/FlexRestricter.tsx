import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './FlexRestricter.module.scss';

export interface FlexRestricterProps {
  vertical?: boolean,
  children?: React.ReactNode,
}

/**
 * Useful when you have some flex layout and need to restrict something within to the limits of layout itself
 *
 * To allow scrollable strictly within flex layout, for example
 */
export const FlexRestricter = React.forwardRef(function FlexRestricter(props: FlexRestricterProps, ref: React.Ref<HTMLDivElement>) {
  const {
    vertical = false,
    children,
  } = props;

  const rootClassName = clsx(s.root, {
    [s.vertical]: vertical,
    [s.horizontal]: !vertical,
  });

  return (
    <div ref={ref} className={rootClassName}>
      {children}
    </div>
  );
});
