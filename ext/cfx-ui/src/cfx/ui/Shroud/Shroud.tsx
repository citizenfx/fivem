import React from "react";
import { useWindowResize } from "cfx/utils/hooks";
import { PreTitleOutlet } from "../outlets";
import { ui } from "../ui";
import s from './Shroud.module.scss';

export interface ShroudProps {
  forRef: React.RefObject<HTMLElement>,
}

export const Shroud = React.forwardRef((props: ShroudProps, ref: React.RefObject<HTMLDivElement>) => {
  const {
    forRef,
  } = props;

  const [posStyle, setPosStyle] = React.useState({});

  const calcPos = React.useCallback(() => {
    if (!forRef.current) {
      return;
    }

    const rect = forRef.current.getBoundingClientRect();

    const vals = {
      x: rect.x,
      y: rect.y,
      w: rect.width,
      h: rect.height,
      sw: globalThis.screen.availWidth,
      sh: globalThis.screen.availHeight,
    };

    setPosStyle(Object.fromEntries(Object.entries(vals).flatMap(([name, value]) =>   [[`--${name}`, ui.px(value)], [`--${name}-raw`, value]]   )));
  }, []);

  React.useEffect(calcPos, []);
  useWindowResize(calcPos);

  return (
    <PreTitleOutlet>
      <div ref={ref} className={s.root} style={posStyle}>
        <div className={s.tile} data-top />
        <div className={s.tile} data-left />
        <div className={s.tile} data-right />
        <div className={s.tile} data-bottom />
      </div>
    </PreTitleOutlet>
  );
});
