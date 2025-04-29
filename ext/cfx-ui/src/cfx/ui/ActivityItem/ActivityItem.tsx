import React from 'react';
import format from 'date-fns/format';
import formatDistanceToNow from 'date-fns/formatDistanceToNow';
import { Flex } from 'cfx/ui/Layout/Flex/Flex';
import { IActivityItem, IActivityItemMedia } from 'cfx/common/services/activity/types';
import { Title } from '../Title/Title';
import { Text } from '../Text/Text';
import { Avatar } from '../Avatar/Avatar';
import { Box } from '../Layout/Box/Box';
import { useBlurhash } from 'cfx/utils/useBlurhash';
import { $L } from 'cfx/common/services/intl/l10n';
import { Pad } from '../Layout/Pad/Pad';
import s from './ActivityItem.module.scss';
import { useActivityItemContext } from './ActivityItem.context';

export interface ActivityItemProps {
  item: IActivityItem,
}

export function ActivityItem(props: ActivityItemProps) {
  const { item } = props;

  // #TODOLOC
  const formattedDate = format(item.date, 'MM/dd/yyyy @ h:mma');

  return (
    <Flex gap="small">
      <Avatar size="small" url={item.userAvatarUrl} />

      <Box grow>
        <Pad right size="large">
          <Flex vertical gap="small">
            <Title delay={200} fixedOn="bottom-left" title={item.userScreenName}>
              <Text weight='bold' opacity="75">
                {item.userDisplayName}
              </Text>
            </Title>

            <Title delay={200} fixedOn='bottom-left' title={<>{formattedDate}<br />{$L('#Feed_OpenInBrowser')}</>}>
              <Text opacity="50">
                <a href={item.url}>
                  {formatDistanceToNow(item.date, { addSuffix: true })}
                </a>
              </Text>
            </Title>

            <div className={s.content}>
              {item.content}
            </div>

            {item.media.map((media) => (
              <Media key={media.id} media={media} />
            ))}
          </Flex>
        </Pad>
      </Box>
    </Flex>
  );
}

function Media({ media }: { media: IActivityItemMedia }) {
  const [hovered, setHovered] = React.useState(false);

  const handleMouseOver = React.useCallback(() => setHovered(true), []);
  const handleMouseLeave = React.useCallback(() => setHovered(false), []);

  let view: React.ReactNode = null;

  switch (media.type) {
    case 'animated_gif':
    case 'photo':
    case 'youtube': {
      view = (
        <ImagePreview media={media} />
      );
      break;
    }

    case 'video': {
      view = (
        <VideoPreview
          media={media}
          hovered={hovered}
        />
      );
      break;
    }
  }

  const previewURL = usePreviewURL(media);

  const style: any = {
    '--aspect-ratio': media.previewAspectRatio,
    backgroundImage: previewURL
      ? `url(${previewURL})`
      : '',
  };

  return (
    <div
      style={style}
      className={s.media}
      onMouseOver={handleMouseOver}
      onMouseLeave={handleMouseLeave}
    >
      {view}
    </div>
  );
}

function ImagePreview({ media }: { media: IActivityItemMedia }) {
  const context = useActivityItemContext();

  const ref = React.useRef<HTMLImageElement | null>(null);

  const handleClick = () => {
    context.showFull(media, ref);
  };

  return (
    <img
      ref={ref}
      src={media.previewUrl}
      onClick={handleClick}
      loading="lazy"
    />
  );
}

function VideoPreview({ media, hovered }: { media: IActivityItemMedia, hovered: boolean }) {
  const context = useActivityItemContext();

  const ref = React.useRef<HTMLVideoElement | null>(null);

  const handleClick = () => {
    context.showFull(media, ref);
  };

  React.useEffect(() => {
    const video = ref.current;

    if (video instanceof HTMLVideoElement) {
      if (video.readyState < video.HAVE_CURRENT_DATA) {
        return;
      }

      if (hovered && video.paused) {
        video.play();
      } else {
        video.pause();
      }
    }
  }, [hovered]);

  return (
    <video
      loop
      muted
      ref={ref}
      src={media.fullUrl}
      onClick={handleClick}
    />
  );
}

function usePreviewURL(media: IActivityItemMedia): string {
  if (media.blurhash) {
    return useBlurhash(media.blurhash, 128, 128) || '';
  }

  if (media.type === 'photo') {
    return '';
  }

  return media.previewUrl || '';
}
