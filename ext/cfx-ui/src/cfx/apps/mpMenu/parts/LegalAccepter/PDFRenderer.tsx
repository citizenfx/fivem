/* eslint-disable react/no-unescaped-entities */
import {
  Icons,
  Flex,
  Text,
  Title,
} from '@cfx-dev/ui-components';
import * as pdfjsLib from 'pdfjs-dist';
import * as pdfjsViewer from 'pdfjs-dist/web/pdf_viewer';
import React from 'react';

import { LinkButton } from 'cfx/ui/Button/LinkButton';
import { useTimeoutFlag } from 'cfx/utils/hooks';

import 'pdfjs-dist/web/pdf_viewer.css';

import s from './PDFRenderer.module.scss';

pdfjsLib.GlobalWorkerOptions.workerSrc = new URL('pdfjs-dist/build/pdf.worker.min.js', import.meta.url).toString();

async function loadAndRender(src: string, container: HTMLDivElement, abortSignal: AbortSignal) {
  // Setting up the container
  {
    container.innerHTML = '';

    const pageViewerNode = document.createElement('div');
    pageViewerNode.classList.add('pdfViewer');

    container.appendChild(pageViewerNode);
  }

  const loadingTask = pdfjsLib.getDocument(src);

  const doc = await loadingTask.promise;

  if (abortSignal.aborted) {
    doc.destroy();

    return;
  }

  const eventBus = new pdfjsViewer.EventBus();
  const l10n = pdfjsViewer.NullL10n;
  const linkService = new pdfjsViewer.SimpleLinkService();

  const viewer = new pdfjsViewer.PDFViewer({
    container,
    eventBus,
    l10n,
    linkService,
  });

  const adjustDocumentScale = () => {
    viewer.currentScaleValue = 'page-width';
  };

  window.addEventListener('resize', adjustDocumentScale);
  eventBus.on('pagesinit', adjustDocumentScale);

  abortSignal.addEventListener('abort', () => {
    window.removeEventListener('resize', adjustDocumentScale);
    eventBus.off('pagesinit', adjustDocumentScale);

    requestIdleCallback(() => {
      viewer.cleanup();
      doc.destroy();
    });
  });

  viewer.setDocument(doc);
}

export interface PDFRendererProps {
  src: string;
}

export function PDFRenderer(props: PDFRendererProps) {
  const {
    src,
  } = props;

  const ref = React.useRef<HTMLDivElement>(null);

  React.useEffect(() => {
    if (!ref.current) {
      return;
    }

    const abortController = new AbortController();

    loadAndRender(src, ref.current, abortController.signal);

    return () => {
      abortController.abort();
    };
  }, [src]);

  return (
    <div className={s.root}>
      <DelayedPlaceholder {...props} />

      <div ref={ref} className={s.wrapper} />
    </div>
  );
}

function DelayedPlaceholder(props: PDFRendererProps) {
  const {
    src,
  } = props;

  const shouldShow = useTimeoutFlag(500);

  if (!shouldShow) {
    return null;
  }

  return (
    <Flex fullWidth fullHeight centered vertical gap="xlarge">
      <Text size="large">It seems like we couldn't load the document for you.</Text>

      <Title title={src}>
        <LinkButton to={src} icon={Icons.externalLink} text="Open the document in your browser" />
      </Title>
    </Flex>
  );
}
