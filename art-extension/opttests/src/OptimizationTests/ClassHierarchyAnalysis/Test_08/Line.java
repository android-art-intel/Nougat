/*
 * Copyright (C) 2016 Intel Corporation
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
 */

package OptimizationTests.ClassHierarchyAnalysis.Test_08;

public class Line extends Move {
    int startx;
    int starty;

    public Line(int x, int y) {
        startx = x;
        starty = y;
    }

    public int left(int shift) {
       startx = startx - shift;
       return startx; 
    }
    public int right(int shift) {
       startx = startx + shift;
       return startx;
    }
}

