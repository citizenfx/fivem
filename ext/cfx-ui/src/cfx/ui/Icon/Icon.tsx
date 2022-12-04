import { clsx } from "cfx/utils/clsx";
import React from "react";
import { TextColor, TextOpacity, TextSize, TEXT_OPACITY_MAP } from "../Text/Text";
import { ui } from "../ui";
import s from './Icon.module.scss';

export interface IconProps {
  color?: TextColor,
  size?: TextSize,
  opacity?: TextOpacity,

  className?: string,

  children: React.ReactNode,
}
export const Icon = React.forwardRef(function Icon(props: IconProps, ref: React.Ref<HTMLDivElement>) {
  const {
    color = 'inherit',
    size = 'normal',
    opacity = '100',

    className,

    children,
  } = props;

  const rootClassName = clsx(s.root, className);
  const style = {
    color: ui.color(color),
    fontSize: ui.fontSize(size),
    opacity: TEXT_OPACITY_MAP[opacity],
  };

  return (
    <div ref={ref} style={style} className={rootClassName}>
      {children}
    </div>
  );
})
