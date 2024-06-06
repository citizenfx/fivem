/**
 * *Mostly* copied from Angulartics2
 *
 * @see https://github.com/angulartics/angulartics2/blob/5eeac98662b0780a5bee0b5d27e0dbe1ac49eaba/src/lib/providers/matomo/matomo.ts
 */

import { inject, injectable, optional } from 'inversify';

import { IAccountService } from 'cfx/common/services/account/account.service';
import { ASID } from 'cfx/utils/asid';

import { AnalyticsProvider } from '../analytics.extensions';
import { IAnalyticsEvent } from '../types';

declare let _paq: any;

@injectable()
export class MatomoAnalyticsProvider implements AnalyticsProvider {
  constructor(
    @inject(IAccountService)
    @optional()
    protected readonly accountService: IAccountService | undefined,
  ) {
    this.initializeUserID();
  }

  private async initializeUserID() {
    this.accountService?.accountChange.addListener((event) => {
      if (event.account) {
        this.setUserId(event.account.id);
      }
    });

    // Fall back to ASID otherwise
    this.setUserId(ASID);
  }

  private setUserId(userID: unknown) {
    try {
      _paq.push(['setUserId', `${userID}`]);
    } catch (e) {
      // ignore everything in here
    }
  }

  trackEvent(event: IAnalyticsEvent): void {
    const {
      action,
      properties,
    }: {
      action: EventTrackAction;
      properties: EventTrackactionProperties;
    } = event;

    let params: [string?, ...any] = [];

    switch (action) {
      /**
       * @description Sets the current page view as a product or category page view. When you call
       * setEcommerceView it must be followed by a call to trackPageView to record the product or
       * category page view.
       *
       * @link https://matomo.org/docs/ecommerce-analytics/#tracking-product-page-views-category-page-views-optional
       * @link https://developer.matomo.org/api-reference/tracking-javascript#ecommerce
       *
       * @property productSKU (required) SKU: Product unique identifier
       * @property productName (optional) Product name
       * @property categoryName (optional) Product category, or array of up to 5 categories
       * @property price (optional) Product Price as displayed on the page
       */
      case 'setEcommerceView':
        params = [
          'setEcommerceView',
          (properties as SetEcommerceViewMatomoProperties).productSKU,
          (properties as SetEcommerceViewMatomoProperties).productName,
          (properties as SetEcommerceViewMatomoProperties).categoryName,
          (properties as SetEcommerceViewMatomoProperties).price,
        ];
        break;

      /**
       * @description Adds a product into the ecommerce order. Must be called for each product in
       * the order.
       *
       * @link https://matomo.org/docs/ecommerce-analytics/#tracking-ecommerce-orders-items-purchased-required
       * @link https://developer.matomo.org/api-reference/tracking-javascript#ecommerce
       *
       * @property productSKU (required) SKU: Product unique identifier
       * @property productName (optional) Product name
       * @property categoryName (optional) Product category, or array of up to 5 categories
       * @property price (recommended) Product price
       * @property quantity (optional, default to 1) Product quantity
       */
      case 'addEcommerceItem':
        params = [
          'addEcommerceItem',
          (properties as AddEcommerceItemProperties).productSKU,
          (properties as AddEcommerceItemProperties).productName,
          (properties as AddEcommerceItemProperties).productCategory,
          (properties as AddEcommerceItemProperties).price,
          (properties as AddEcommerceItemProperties).quantity,
        ];
        break;

      /**
       * @description Tracks a shopping cart. Call this javascript function every time a user is
       * adding, updating or deleting a product from the cart.
       *
       * @link https://matomo.org/docs/ecommerce-analytics/#tracking-add-to-cart-items-added-to-the-cart-optional
       * @link https://developer.matomo.org/api-reference/tracking-javascript#ecommerce
       *
       * @property grandTotal (required) Cart amount
       */
      case 'trackEcommerceCartUpdate':
        params = ['trackEcommerceCartUpdate', (properties as TrackEcommerceCartUpdateMatomoProperties).grandTotal];
        break;

      /**
       * @description Tracks an Ecommerce order, including any ecommerce item previously added to
       * the order. orderId and grandTotal (ie. revenue) are required parameters.
       *
       * @link https://matomo.org/docs/ecommerce-analytics/#tracking-ecommerce-orders-items-purchased-required
       * @link https://developer.matomo.org/api-reference/tracking-javascript#ecommerce
       *
       * @property orderId (required) Unique Order ID
       * @property grandTotal (required) Order Revenue grand total (includes tax, shipping, and subtracted discount)
       * @property subTotal (optional) Order sub total (excludes shipping)
       * @property tax (optional) Tax amount
       * @property shipping (optional) Shipping amount
       * @property discount (optional) Discount offered (set to false for unspecified parameter)
       */
      case 'trackEcommerceOrder':
        params = [
          'trackEcommerceOrder',
          (properties as TrackEcommerceOrderMatomoProperties).orderId,
          (properties as TrackEcommerceOrderMatomoProperties).grandTotal,
          (properties as TrackEcommerceOrderMatomoProperties).subTotal,
          (properties as TrackEcommerceOrderMatomoProperties).tax,
          (properties as TrackEcommerceOrderMatomoProperties).shipping,
          (properties as TrackEcommerceOrderMatomoProperties).discount,
        ];
        break;

      /**
       * @description To manually trigger an outlink
       *
       * @link https://matomo.org/docs/tracking-goals-web-analytics/
       * @link https://developer.matomo.org/guides/tracking-javascript-guide#tracking-a-click-as-an-outlink-via-css-or-javascript
       *
       * @property url (required) link url
       * @property linkType (optional) type of link
       */
      case 'trackLink':
        params = [
          'trackLink',
          (properties as TrackLinkMatomoProperties).url,
          (properties as TrackLinkMatomoProperties).linkType,
        ];
        break;

      /**
       * @description Tracks an Ecommerce goal
       *
       * @link https://matomo.org/docs/tracking-goals-web-analytics/
       * @link https://developer.matomo.org/guides/tracking-javascript-guide#manually-trigger-goal-conversions
       *
       * @property goalId (required) Unique Goal ID
       * @property value (optional) passed to goal tracking
       */
      case 'trackGoal':
        params = [
          'trackGoal',
          (properties as TrackGoalMatomoProperties).goalId,
          (properties as TrackGoalMatomoProperties).value,
        ];
        break;

      /**
       * @description Tracks a site search
       *
       * @link https://matomo.org/docs/site-search/
       * @link https://developer.matomo.org/guides/tracking-javascript-guide#internal-search-tracking
       *
       * @property keyword (required) Keyword searched for
       * @property category (optional) Search category
       * @property searchCount (optional) Number of results
       */
      case 'trackSiteSearch':
        params = [
          'trackSiteSearch',
          (properties as TrackSiteSearchMatomoProperties).keyword,
          (properties as TrackSiteSearchMatomoProperties).category,
          (properties as TrackSiteSearchMatomoProperties).searchCount,
        ];
        break;

      /**
       * @description Logs an event with an event category (Videos, Music, Games...), an event
       * action (Play, Pause, Duration, Add Playlist, Downloaded, Clicked...), and an optional
       * event name and optional numeric value.
       *
       * @link https://matomo.org/docs/event-tracking/
       * @link https://developer.matomo.org/api-reference/tracking-javascript#using-the-tracker-object
       *
       * @property category
       * @property action
       * @property name (optional, recommended)
       * @property value (optional)
       */
      default:
        // PAQ requires that eventValue be an integer, see: http://matomo.org/docs/event-tracking
        if ((properties as TrackEventMatomoProperties).value) {
          const parsed = parseInt((properties as TrackEventMatomoProperties).value as any, 10);
          (properties as TrackEventMatomoProperties).value = Number.isNaN(parsed)
            ? 0
            : parsed;
        }

        params = [
          'trackEvent',
          (properties as TrackEventMatomoProperties).category,
          action,
          (properties as TrackEventMatomoProperties).name || (properties as TrackEventMatomoProperties).label, // Changed in favour of Matomo documentation. Added fallback so it's backwards compatible.
          (properties as TrackEventMatomoProperties).value,
        ];
    }

    try {
      _paq.push(params);
    } catch (e) {
      if (!(e instanceof ReferenceError)) {
        throw e;
      }
    }
  }
}

export type EventTrackAction =
  | 'setEcommerceView'
  | 'addEcommerceItem'
  | 'trackEcommerceCartUpdate'
  | 'trackEcommerceOrder'
  | 'trackLink'
  | 'trackGoal'
  | 'trackSiteSearch'
  | string;

export type ScopeMatomo = 'visit' | 'page';

export interface DimensionsMatomoProperties {
  dimension0?: string;
  dimension1?: string;
  dimension2?: string;
  dimension3?: string;
  dimension4?: string;
  dimension5?: string;
  dimension6?: string;
  dimension7?: string;
  dimension8?: string;
  dimension9?: string;
}
export interface SetEcommerceViewMatomoProperties {
  /** @class SetEcommerceViewMatomoProperties */
  productSKU: string;
  /** @class SetEcommerceViewMatomoProperties */
  productName: string;
  /** @class SetEcommerceViewMatomoProperties */
  categoryName: string;
  /** @class SetEcommerceViewMatomoProperties */
  price: string;
}

export interface AddEcommerceItemProperties {
  /** @class AddEcommerceItemProperties */
  productSKU: string;
  /** @class AddEcommerceItemProperties */
  productName: string;
  /** @class AddEcommerceItemProperties */
  productCategory: string;
  /** @class AddEcommerceItemProperties */
  price: string;
  /** @class AddEcommerceItemProperties */
  quantity: string;
}

export interface TrackEcommerceCartUpdateMatomoProperties {
  /** @class TrackEcommerceCartUpdateMatomoProperties */
  grandTotal: string;
}

export interface TrackEcommerceOrderMatomoProperties {
  /** @class TrackEcommerceOrderMatomoProperties */
  orderId: string;
  /** @class TrackEcommerceOrderMatomoProperties */
  grandTotal: string;
  /** @class TrackEcommerceOrderMatomoProperties */
  subTotal: string;
  /** @class TrackEcommerceOrderMatomoProperties */
  tax: string;
  /** @class TrackEcommerceOrderMatomoProperties */
  shipping: string;
  /** @class TrackEcommerceOrderMatomoProperties */
  discount: string;
}

export interface TrackLinkMatomoProperties {
  /** @class TrackLinkMatomoProperties */
  url: string;
  /** @class TrackLinkMatomoProperties */
  linkType: string;
}

export interface TrackGoalMatomoProperties {
  /** @class TrackGoalMatomoProperties */
  goalId: string;
  /** @class TrackGoalMatomoProperties */
  value: string;
}

export interface TrackSiteSearchMatomoProperties {
  /** @class TrackSiteSearchMatomoProperties */
  keyword: string;
  /** @class TrackSiteSearchMatomoProperties */
  category: string;
  /** @class TrackSiteSearchMatomoProperties */
  searchCount: string;
}

export interface TrackEventMatomoProperties {
  /** @class TrackEventMatomoProperties */
  category: string;
  /** @class TrackEventMatomoProperties */
  name?: string;
  /** @class TrackEventMatomoProperties */
  label?: string;
  /** @class TrackEventMatomoProperties */
  value: number | string;
}

export interface SetCustomVariableMatomoProperties extends DimensionsMatomoProperties {
  /** @class SetCustomVariableMatomoProperties */
  index: number;
  /** @class SetCustomVariableMatomoProperties */
  name: string;
  /** @class SetCustomVariableMatomoProperties */
  value: string;
  /** @class SetCustomVariableMatomoProperties */
  scope: ScopeMatomo;
}

export interface DeleteCustomVariableMatomoProperties {
  /** @class DeleteCustomVariableMatomoProperties */
  index: number;
  /** @class DeleteCustomVariableMatomoProperties */
  scope: ScopeMatomo;
}

export type EventTrackactionProperties =
  | SetEcommerceViewMatomoProperties
  | AddEcommerceItemProperties
  | TrackEcommerceCartUpdateMatomoProperties
  | TrackEcommerceOrderMatomoProperties
  | TrackLinkMatomoProperties
  | TrackGoalMatomoProperties
  | TrackSiteSearchMatomoProperties
  | TrackEventMatomoProperties;

interface Profile {
  name: string;
  tile: string;
  type: string;
  identifier: number;
  externalIdentifier: string;
  parameters: Record<string, string>;
  signedIn: boolean;
}

// eslint-disable-next-line @typescript-eslint/no-unused-vars
interface Profiles {
  profiles: Profile[];
}
