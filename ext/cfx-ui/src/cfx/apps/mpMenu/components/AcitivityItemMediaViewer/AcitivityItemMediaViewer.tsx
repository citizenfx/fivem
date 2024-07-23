import { Interactive, ui, useOutlet, clsx } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { AcitivityItemMediaViewerState, IRect } from './AcitivityItemMediaViewer.state';

import s from './AcitivityItemMediaViewer.module.scss';

export const AcitivityItemMediaViewer = observer(function AcitivityItemMediaViewer() {
  const state = AcitivityItemMediaViewerState;

  const ref = React.useRef<HTMLImageElement | HTMLVideoElement | HTMLIFrameElement | null>(null);
  const [toRect, setToRect] = React.useState<IRect | null>(null);
  const Outlet = useOutlet('acitivityItemMedia');

  React.useEffect(() => {
    if (!state.active) {
      setToRect(null);

      return;
    }

    if (!ref.current) {
      return;
    }

    let inactive = false;
    const onLoad = () => {
      if (inactive || !ref.current) {
        return;
      }

      const rect = ref.current.getBoundingClientRect();

      setToRect({
        x: rect.x,
        y: rect.y,
        w: rect.width,
        h: rect.height,
      });
    };

    const fireImmediately = (ref.current instanceof HTMLImageElement && ref.current.complete)
      || ref.current instanceof HTMLVideoElement;

    if (fireImmediately) {
      onLoad();

      return;
    }
    ref.current.onload = onLoad;

    return () => {
      inactive = true;
    };
  }, [state.active]);

  if (!state.media || !state.fromRect) {
    return null;
  }

  const style: React.CSSProperties = {
    '--tw': ui.px(toRect?.w || 0),
    '--th': ui.px(toRect?.h || 0),
    '--x': ui.px(state.fromRect.x),
    '--y': ui.px(state.fromRect.y),
    '--s': state.fromRect.w / (toRect?.w || 1),
    '--ar': state.media.fullAspectRatio,
  } as any;

  // eslint-disable-next-line no-nested-ternary
  const TargetElement = state.media.type === 'video'
    ? 'video'
    : state.media.type === 'youtube'
      ? 'iframe'
      : 'img';

  return (
    <Outlet>
      <Interactive
        showPointer={false}
        style={style}
        className={clsx(s.root, {
          [s.active]: !!toRect,
        })}
        onClick={state.close}
      >
        <TargetElement loop autoPlay ref={ref as any} src={state.media.fullUrl} className={s.media} />
      </Interactive>
    </Outlet>
  );
});
