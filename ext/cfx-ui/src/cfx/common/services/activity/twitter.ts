import { splitByIndices } from "cfx/utils/string";
import { IActivityItemData, IActivityItemMedia, IRawTweet } from "./types";

export function rawTweetToActivityDataItem(rawTweet: IRawTweet): IActivityItemData | null {
  let actualTweet = rawTweet;
  let repostedBy: string | undefined = undefined;

  if (rawTweet.retweeted_status) {
    actualTweet = rawTweet.retweeted_status;
    repostedBy = actualTweet.user.name;
  }

  const fallbackContent = new Map([[0, actualTweet.text]]);
  let content: Map<number, string>;
  let media: IActivityItemMedia[] = [];

  try {
    if (actualTweet.extended_entities?.media) {
      for (const mediaEntity of actualTweet.extended_entities.media) {
        const previewUrl = mediaEntity.media_url_https || mediaEntity.media_url;

        const mediaItem: IActivityItemMedia = {
          id: mediaEntity.id_str,
          type: mediaEntity.type as any,

          previewAspectRatio: mediaEntity.sizes.large.w / mediaEntity.sizes.large.h,
          previewUrl,

          fullAspectRatio: 1,
          fullUrl: '',
        };

        mediaItem.fullAspectRatio = mediaEntity.sizes.large.w / mediaEntity.sizes.large.h;
        mediaItem.fullUrl = previewUrl;

        if (mediaEntity.type === 'animated_gif' || mediaEntity.type === 'video') {
          mediaItem.fullAspectRatio = mediaEntity.video_info.aspect_ratio[0] / mediaEntity.video_info.aspect_ratio[1];
          mediaItem.fullUrl = mediaEntity.video_info.variants.sort((a, b) => b.bitrate - a.bitrate)[0].url;
        }

        media.push(mediaItem);
      }
    }

    const mediaIndicesFull = (actualTweet.entities.media || []).map((media) => media.indices);
    const mediaIndices = mediaIndicesFull.map(([x]) => x);

    const urlIndicesFull = (actualTweet.entities.urls || []).map((url) => url.indices);

    content = splitByIndices(
      actualTweet.text,
      mediaIndicesFull.flat().concat(urlIndicesFull.flat()),
      true,
    );

    for (const index of mediaIndices) {
      content.delete(index);
    }
  } catch (e) {
    content = fallbackContent;
  }

  try {
    const links = (actualTweet.entities.urls || []).map((link) => ({
      indices: link.indices,
      url: link.display_url,
    } as IActivityItemData['links'][0]));

    const activity: IActivityItemData = {
      id: actualTweet.id_str,
      url: `https://twitter.com/FiveM/status/${actualTweet.id_str}`,
      date: new Date(actualTweet.created_at),
      content,
      media,
      userAvatarUrl: actualTweet.user.profile_image_url_https || actualTweet.user.profile_image_url,
      userDisplayName: actualTweet.user.name,
      userScreenName: '@' + actualTweet.user.screen_name,
      links,
      repostedBy,
    };

    return activity;
  } catch (e) {
    console.warn('Failed to convert raw tweet to activity data item', e);

    return null;
  }
}
