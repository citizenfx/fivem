import { clsx } from "cfx/utils/clsx";
import React from "react";
import { FixedSizeList, ListOnScrollProps } from "react-window";
import { Scrollable } from "./Scrollable";
import s from './Scrollable.module.scss';

export interface VirtualScrollableProps {
  className?: string,
  itemCount: number,
  itemHeight: number,
  renderItem(index: number): React.ReactNode,

  onScrollUpdate?(offset: number): void,
  initialScrollOffset?: number,
}

export function VirtualScrollable(props: VirtualScrollableProps) {
  const {
    className,
    itemCount,
    itemHeight,
    renderItem,
    initialScrollOffset,
  } = props;

  const onScroll = props.onScrollUpdate
    ? (e: ListOnScrollProps) => props.onScrollUpdate?.(e.scrollOffset)
    : undefined;

  const [ref, width, height] = useListSizes();

  return (
    <div ref={ref} className={clsx(s.virtual, className)}>
      <FixedSizeList
        outerElementType={OuterScrollable}
        width={width}
        height={height}
        itemCount={itemCount}
        itemSize={itemHeight}
        initialScrollOffset={initialScrollOffset}
        onScroll={onScroll}
      >
        {({ index, style }) => (
          <div style={style}>
            {renderItem(index)}
          </div>
        )}
      </FixedSizeList>
    </div>
  );
}

const OuterScrollable = React.forwardRef(function OuterScrollable(props, ref: any) {
  return (
    <Scrollable scrollerRef={ref} {...props} />
  );
});

function useListSizes(): [React.Ref<HTMLDivElement>, number, number] {
  const [sizes, setSizes] = React.useState<[number, number]>([500, 500]);
  const ref = React.useRef<HTMLDivElement>(null);

  React.useEffect(() => {
    const resizeHandler = () => {
      if (ref.current) {
        const rect = ref.current.getBoundingClientRect();

        setSizes([rect.width, rect.height]);
      }
    };

    // requestAnimationFrame(resizeHandler);

    resizeHandler();

    window.addEventListener('resize', resizeHandler);

    return () => {
      window.removeEventListener('resize', resizeHandler);
    };
  }, []);

  return [ref, ...sizes];
}
