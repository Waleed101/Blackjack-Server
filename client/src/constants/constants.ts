//base URl
let parts = window.location.href.split(":");
export const URL = parts[0] + ":" + parts[1] + ":3000/";

// polling constant (miliseconds)
export const POLL_REFRESH_INTERVAL = 2000;
