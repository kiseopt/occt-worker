addToLibrary({
  occt_worker_cancelled__sig: "i",
  occt_worker_cancelled: () => {
    const callback = Module["occtWorkerCancelled"];
    return typeof callback === "function" && callback() ? 1 : 0;
  },
  occt_worker_progress__sig: "vd",
  occt_worker_progress: (fraction) => {
    const callback = Module["occtWorkerProgress"];
    if (typeof callback === "function") callback(fraction);
  },
});
