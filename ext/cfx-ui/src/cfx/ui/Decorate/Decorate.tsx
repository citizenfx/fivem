import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './Decorate.module.scss';

export interface DecorateProps {
  decorator: React.ReactNode,

  children?: React.ReactNode,
  className?: string,
  decoratorClassName?: string,
}

export const Decorate = React.forwardRef(function Decorate(props: DecorateProps, ref: React.Ref<HTMLDivElement>) {
  const {
    decorator,

    children,
    className,
  } = props;

  const rootClassName = clsx(s.root, className);
  const decoratorClassName = clsx(s.decorator, props.decoratorClassName);

  return (
    <div ref={ref} className={rootClassName}>
      {children}

      {!!decorator && (
        <div className={decoratorClassName}>
          {decorator}
        </div>
      )}
    </div>
  );
});
