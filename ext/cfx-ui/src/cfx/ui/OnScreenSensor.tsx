import { noop } from "cfx/utils/functional";
import { useDynamicRef } from "cfx/utils/hooks";
import React from "react";

export interface OnScreenSensorProps {
  onEnter?(): void,
  onLeave?(): void,
}

const style = {
  width: '0',
  height: '0',
};

export function OnScreenSensor(props: OnScreenSensorProps) {
  const {
    onEnter = noop,
    onLeave = noop,
  } = props;

  const onEnterRef = useDynamicRef(onEnter);
  const onLeaveRef = useDynamicRef(onLeave);

  const ref = React.useRef(null);

  React.useEffect(() => {
    if (!ref.current) {
      return;
    }

    const observer = new IntersectionObserver((entries) => {
      const [el] = entries;
      if (!el) {
        return;
      }

      if (el.isIntersecting) {
        onEnterRef.current();
      } else {
        onLeaveRef.current();
      }
    });

    observer.observe(ref.current);

    return () => observer.disconnect();
  }, []);

  return (
    <span ref={ref} style={style} />
  );
}
