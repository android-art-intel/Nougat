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

class Main {
  public static void main(String[] args) {
      int fx, fy;
      Rectangle rect = new Rectangle(0,0);
      Line line = new Line(10,10);

      fx = line.left(20);
      System.out.println(" X = " + fx);
      fx = line.right(40);
      System.out.println(" Y = " + fx);

      fx = rect.right(25);
      fy = rect.up(45);
      System.out.println(" X = " + fx);
      System.out.println(" Y = " + fy);
      fx = rect.left(25);
      fy = rect.down(45);
      System.out.println(" X = " + fx);
      System.out.println(" Y = " + fy);
  }
}
