/*
 * Copyright (C) 2015 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package com.intel.robo;

import java.util.ArrayList;

public abstract class TestBenchBase extends TestBase {

    protected int warmups = 0;
    protected int runs = 5;
    protected int measures = 3;
    protected int accuracy = 3;
    protected int reruns = 10;
    protected int currentRun = 0;
    protected int workloadRevision = 1;
    protected String workloadName = "";
    protected String runCustom = "";
    protected String resultsPrefix = "[result scores] ";
    protected String scoreSuffix = "";
    protected String mainScores;
    protected String mainScore;
    protected String[] resultNames;
    protected String[] resultIDs;
    protected ArrayList<String>[] results;

    void clearResults() {
        results = Utils.makeStringArrayList(resultNames.length);
    }

    @Override
    public void dumpBaseParameters() {
        super.dumpBaseParameters();
        log("Benhcmark parameters: warmups: " + warmups + ", runs: " + runs + ", reruns: " + reruns + ", measures: " + measures + ", accuracy: " + accuracy
                + ", runCustom: " + runCustom);
    }

    @Override
    public void loadParameters() {
        super.loadParameters();
        runs = getSysPropInt("runs", runs);
        reruns = getSysPropInt("reruns", reruns);
        warmups = getSysPropInt("warmups", warmups);
        measures = getSysPropInt("measures", measures);
        accuracy = getSysPropInt("accuracy", accuracy);
        currentRun = getSysPropInt("currentRun", currentRun);
        workloadName = getSysPropStr("workloadName", workloadName);
        workloadRevision = getSysPropInt("workloadRevision", workloadRevision);
        runCustom = getSysPropStr("runCustom", runCustom);
        resultsPrefix = getSysPropStr("resultsPrefix", resultsPrefix);
        scoreSuffix = getSysPropStr("scoreSuffix", scoreSuffix);
    }

    boolean checkResults() {
        return Utils.checkResults(results, resultNames, measures, accuracy, mainScore, mainScores);
    }

    public String getWorkloadName() {
        return workloadName != null && workloadName.length() > 0 ? workloadName : getName();
    }

    public String getWorkloadNameRev() {
        return workloadRevision >= 0 ? getWorkloadName() + "/" + workloadRevision : getWorkloadName();
    }

    public void printResults() {
        Utils.printResults(getWorkloadNameRev(), results, resultNames, resultsPrefix, scoreSuffix, measures, mainScore);
    }

    void runBenchLoop() throws Exception {
        // warm up
        log("Expected warm-ups " + warmups + ", current run time: " + currentRunTime());
        for (int i = 0; i < warmups && hasTime(); i++) {
            if (currentRun > 1) {
                currentRun--;
                log("Skipping warmup #" + (i + 1));
                continue;
            }
            log("Run #" + (i + 1) + ", current run time: " + currentRunTime() + " (warm-up)");
            runBench(0, false);
        }

        int runIteration = 0;

        // primary results
        log("Expected runs " + runs + ", current run time: " + currentRunTime());
        for (int i = 0; i < runs && hasTime(); i++) {
            runIteration++;
            if (currentRun > 1) {
                currentRun--;
                log("Skipping run #" + runIteration + "...");
                continue;
            }
            log("Run #" + runIteration + ", current run time: " + currentRunTime() + " (primary)");
            startGCProf();
            runBench(runIteration, true);
            finishGCProf(getWorkloadName() + "_run" + runIteration);
        }

        // accuracy re-runs
        log("Expected reruns " + reruns + ", current run time: " + currentRunTime());
        boolean checkResults = true;
        for (int i = 0; i < reruns && hasTime(); i++) {
            // check results each second time
            if (checkResults && checkResults())
                break;
            checkResults = !checkResults;
            runIteration++;
            if (currentRun > 1) {
                currentRun--;
                log("Skipping rerun #" + runIteration + "...");
                continue;
            }
            log("Run #" + runIteration + ", current run time: " + currentRunTime() + " (accuracy rerun)");
            startGCProf();
            runBench(runIteration, true);
            finishGCProf(getWorkloadName() + "_run" + runIteration);
        }

        if (!hasTime()) {
            log("Out of time: " + currentRunTime() + " of " + maxRunTime);
        }
    }

    public void testBenchMain() {
        try {
            log("BenchMain started...");
            if (collectLogcat) {
                startLogcat();
            }
            clearResults();
            startBench();
            runBenchLoop();
            stopLogcat();
            printResults();
            status = "passed";
        } catch (Exception e) {
            screenshot(getWorkloadName() + "-screenshot-FAILED");
            dump();
            log("BenchMain error", e);
        }
        log("BenchMain finished: " + status);
    }

    abstract public void startBench() throws Exception;
    abstract public void runBench(int iteration, boolean collectScores) throws Exception;
}
