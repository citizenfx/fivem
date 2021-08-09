import React from 'react';
import classnames from 'classnames';
import { createOutlet } from 'utils/outlets';
import { div } from 'utils/styled';
import { makeAutoObservable } from 'mobx';
import { observer } from 'mobx-react-lite';
import { clamp } from 'shared/math';
import s from './Intro.module.scss';

const OFFSET = 40;
const FOCUSER_OFFSET = 32;
const FOCUSER_OFFSET_HALF = FOCUSER_OFFSET / 2;

export interface IntroStep {
  /**
   * Content of target's data-intro-id attribute
   */
  focusElement?: string,

  content: React.FC,

  nextButtonText?: string,

  onEnter?(): void,
  onExit?(): void,
}

export interface IntroProps {
  steps: IntroStep[],

  onFinish(): void,
}

const Root = div(s, 'root');
const IntroOutlet = createOutlet();

interface FocusNodeRect {
  x: number,
  y: number,
  w: number,
  h: number,
}
interface FocusNode {
  id: string,
  rect: FocusNodeRect,
  borderRadius: string
}

export const Intro = observer(function Intro(props: IntroProps) {
  const {
    steps,
    onFinish,
  } = props;

  const [stepIndex, setStepIndex] = React.useState(0);

  const step = steps[stepIndex];
  const stepNode = useStepNode(step.focusElement);
  const lastStep = stepIndex === (steps.length - 1);

  useResizeObserver();

  React.useEffect(() => {
    step.onEnter?.();

    return step.onExit;
  }, [step]);

  const handleStepForward = () => {
    const nextStepIndex = stepIndex + 1;

    if (nextStepIndex === steps.length) {
      onFinish();
    } else {
      setStepIndex(nextStepIndex);
    }
  };

  const handleStepBackward = () => {
    const nextStepIndex = stepIndex - 1;

    if (nextStepIndex < 0) {
      setStepIndex(0);
    } else {
      setStepIndex(nextStepIndex);
    }
  };

  return (
    <IntroOutlet>
      <Root>
        <Focuser node={stepNode} />
        <Content index={stepIndex} step={step} node={stepNode}>
          {!lastStep && (
            <button onClick={onFinish}>
              Close
            </button>
          )}

          {lastStep && (
            <button data-back onClick={handleStepBackward}>
              Back
            </button>
          )}

          <div>
            {!lastStep && (
              <button data-back onClick={handleStepBackward}>
                Back
              </button>
            )}

            <button data-success onClick={handleStepForward}>
              {step.nextButtonText || 'Next'}
            </button>
          </div>
        </Content>
      </Root>
    </IntroOutlet>
  );
});

function Focuser({ node }: { node: FocusNode | void }) {
  const [style, setStyle] = React.useState<React.CSSProperties>({});

  React.useEffect(() => {
    if (node) {
      const { rect, borderRadius } = node;

      setStyle({
        transform: `translate(${rect.x - FOCUSER_OFFSET_HALF}px, ${rect.y - FOCUSER_OFFSET_HALF}px)`,
        width: `${rect.w + FOCUSER_OFFSET}px`,
        height: `${rect.h + FOCUSER_OFFSET}px`,
        borderRadius,
      });
    } else {
      setStyle({
        transform: 'translate(-10px, -10px)',
        width: '0px',
        height: '0px',
        borderRadius: '0px',
      });
    }
  }, [node]);

  return (
    <div className={s.focuser} style={style} />
  );
}

function Content({ step, index, node, children }: { step: IntroStep, index: number, node: FocusNode | void, children?: React.ReactNode }) {
  const ref = React.useRef<HTMLDivElement>();
  const style = useContentStyle(ref, step, node);

  const ContentNode = step.content;

  const contentClassName = classnames(s.content, {
    [s.transitions]: index !== 0,
  });

  return (
    <div ref={ref} className={contentClassName} style={style}>
      <div className={s.wrapper}>
        <ContentNode />
      </div>

      <div className={s.controls}>
        {children}
      </div>
    </div>
  );
}

function getFocusNode(id: string | void): FocusNode | void {
  if (!id) {
    return;
  }

  const nodes = [...document.querySelectorAll(`[data-intro-id="${id}"]`)];
  if (nodes.length === 0) {
    return;
  }

  const nodeRects = nodes.map(getFocusNodeRect);

  const x = Math.min(...nodeRects.map(({ x }) => x));
  const y = Math.min(...nodeRects.map(({ y }) => y));

  const maxBottom = Math.max(...nodeRects.map(({ y, h }) => y + h));
  const maxRight = Math.max(...nodeRects.map(({ x, w }) => x + w));

  const rect: FocusNodeRect = {
    x,
    y,
    w: maxRight - x,
    h: maxBottom - y,
  };

  let borderRadius = nodes.length === 1
    ? getComputedStyle(nodes[0]).borderRadius
    : '4px';

  if (borderRadius === '0px') {
    borderRadius = '4px';
  }

  return {
    id,
    rect,
    borderRadius,
  };
}

function getFocusNodeRect(node: Element): FocusNodeRect {
  const rect = node.getBoundingClientRect();

  return {
    x: rect.x,
    y: rect.y,
    w: rect.width,
    h: rect.height,
  };
}

const recalculations = new class {
  public counter = 0;

  constructor() {
    makeAutoObservable(this);
  }

  increment() {
    this.counter++;
  }
};

function useStepNode(id: string | void): FocusNode | void {
  const [node, setNode] = React.useState<FocusNode | void>();

  React.useLayoutEffect(() => {
    setNode(getFocusNode(id));
  }, [
    id,
    recalculations.counter, // so we can recalculate focus node if something signals to do so
  ]);

  return node;
}

export function useIntroRecalculate(...deps: React.DependencyList) {
  React.useEffect(() => {
    recalculations.increment();
  }, deps);
}

export function IntroForceRecalculate() {
  useIntroRecalculate();

  return null;
}

function useResizeObserver() {
  React.useEffect(() => {
    const resizeObserver = new ResizeObserver(() => recalculations.increment());

    resizeObserver.observe(document.body);

    return () => resizeObserver.disconnect();
  }, []);
}

function useContentStyle(ref: React.RefObject<HTMLDivElement>, step: IntroStep, node: FocusNode | void): React.CSSProperties {
  const [pos, setPos] = React.useState<[number, number]>([0, 0]);

  React.useLayoutEffect(() => {
    if (!ref.current) {
      return;
    }

    const {
      w: refWidth,
      h: refHeight,
    } = getFocusNodeRect(ref.current);

    const bodyRect = getFocusNodeRect(document.body);

    const nodeRect = node
      ? node.rect
      : bodyRect;

    const nodeWidth = nodeRect.w;
    const nodeHeight = nodeRect.h;
    const nodeTop = nodeRect.y;
    const nodeLeft = nodeRect.x;
    const nodeBottom = nodeTop + nodeHeight;
    const nodeRight = nodeLeft + nodeWidth;

    const boundaryWidth = bodyRect.w - (OFFSET * 2);
    const boundaryHeight = bodyRect.h - (OFFSET * 2);
    const boundaryTop = OFFSET;
    const boundaryLeft = OFFSET;
    const boundaryBottom = bodyRect.h - OFFSET;
    const boundaryRight = bodyRect.w - OFFSET;

    let x = clamp(nodeLeft + (nodeWidth / 2) - (refWidth / 2), boundaryLeft, boundaryRight - refWidth);
    let y = clamp(nodeTop + (nodeHeight / 2) - (refHeight / 2), boundaryTop, boundaryBottom - refHeight);

    const insideX = boundaryWidth < (nodeWidth + refWidth + OFFSET);
    const insideY = boundaryHeight < (nodeHeight + refHeight + OFFSET);

    const afterX = boundaryRight >= (nodeRight + refWidth + OFFSET);
    const afterY = boundaryBottom >= (nodeBottom + refHeight + OFFSET);

    if (insideY) {
      if (!insideX) {
        x = afterX
          ? nodeWidth + OFFSET
          : nodeLeft - refWidth - OFFSET;
      }
    } else {
      y = afterY
        ? nodeBottom + OFFSET
        : nodeTop - refHeight - OFFSET;
    }

    setPos([x, y]);
  }, [node, ref.current, step]); // step is here so if two sequential step nodes don't have, uh, nodes we we'll recalculate

  return {
    transform: `translate(${pos[0]}px, ${pos[1]}px)`,
  };
}
