import CONFIG from './config';
import { defineComponent, PropType } from 'vue';

export interface SuggestionParam {
  name: string;
  disabled: boolean;
  help: string;
}

export interface Suggestion {
  name: string;
  help: string;
  params: SuggestionParam[];

  disabled: boolean;
}

export default defineComponent({
  props: {
    message: {
      type: String,
      required: true,
    },
    
    suggestions: {
      type: Array as PropType<Suggestion[]>,
      required: true,
    }
  },
  data() {
    return {};
  },
  computed: {
    currentSuggestions(): Suggestion[] {
      if (this.message === '') {
        return [];
      }
      const currentSuggestions = this.suggestions.filter((s) => {
        if (!s.name.startsWith(this.message)) {
          const suggestionSplitted = s.name.split(' ');
          const messageSplitted = this.message.split(' ');
          for (let i = 0; i < messageSplitted.length; i += 1) {
            if (i >= suggestionSplitted.length) {
              return i < suggestionSplitted.length + s.params.length;
            }
            if (suggestionSplitted[i] !== messageSplitted[i]) {
              return false;
            }
          }
        }
        return true;
      }).slice(0, CONFIG.suggestionLimit);

      currentSuggestions.forEach((s) => {
        // eslint-disable-next-line no-param-reassign
        s.disabled = !s.name.startsWith(this.message);

        s.params.forEach((p, index) => {
          const wType = (index === s.params.length - 1) ? '.' : '\\S';
          const regex = new RegExp(`${s.name} (?:\\w+ ){${index}}(?:${wType}*)$`, 'g');

          // eslint-disable-next-line no-param-reassign
          // @ts-ignore
          p.disabled = this.message.match(regex) == null;
        });
      });
      return currentSuggestions;
    },
  },
  methods: {},
});
