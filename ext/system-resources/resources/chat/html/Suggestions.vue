<template>
    <div class="suggestions-wrap" v-show="currentSuggestions.length > 0">
        <ul class="suggestions">
        <li class="suggestion" v-for="s in currentSuggestions" :key="s.name">
            <p>
            <span :class="{ 'disabled': s.disabled }">
                {{s.name}}
            </span>
            <span class="param"
                    v-for="p in s.params"
                    :class="{ 'disabled': p.disabled }"
                    :key="p.name">
                [{{p.name}}]
            </span>
            </p>
            <small class="help">
            <template v-if="!s.disabled">
                {{s.help}}
            </template>
            <template v-for="p in s.params" v-if="!p.disabled">
                {{p.help}}
            </template>
            </small>
        </li>
        </ul>
    </div>
</template>

<script lang="ts" src="./Suggestions.ts"></script>