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

package OptimizationTests.TrivialLoopEvaluator.TryCatchTests;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Comparator;

public class Main {

    public static final int maxIter=999;
    public static final int iterations=100;

    static int inlined(int testVar) {
        int additionalVar = 1;
        for (int i = 0; i < iterations; i++) {
            testVar += 5;
            additionalVar += i + 2*i - i*7 + i*4 + i*5 + i*6*i;
        }
        return testVar + additionalVar;
    }

    public static int inlineIntAndShlXor(int a, int b){
        int ret=0;
        ret&=a<<b;
        ret^=2;
        return ret;
    }
    public static int inlineIntAndShlXortc1(int a, int b){
        int ret=0;
        try {
            ret&=a<<b;
            ret^=2;
        } catch (Exception e) {
            ret+=1;
        }
        return ret;
    }
    public static int inlineIntAndShlXortc2(int a, int b){
        int ret=0;
        try {
            ret&=a<<b;
            ret^=2;
            ret/=49-b;
        } catch (Exception e) {
            ret+=1;
        }
        return ret;
    }
    public static int inlineIntAndShlXortc3(int a, int b){
        int ret=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            ret&=a<<b;
            ret^=2;
        }
        return ret;
    }
    public static int inlineIntAndShlXortc4(int a, int b){
        int ret=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            ret++;
        } finally {
            ret&=a<<b;
            ret^=2;
        }
        return ret;
    }
    public static int inlineIntAndShlXortc5(int a, int b){
        int ret=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            ret++;
        }
        ret&=a<<b;
        ret^=2;
        return ret;
    }
    public static int inlineIntAndShlXortc6(int a, int b){
        int ret=0;
        ret&=a<<b;
        ret^=2;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            ret++;
        }
        return ret;
    }
    public static void main(String[] args) {
        Main main = new Main();
        Class<Main> cls = Main.class;
        Method[] mets = cls.getDeclaredMethods();
        Arrays.sort(mets, new Comparator<Method>() {    
            @Override
            public int compare(Method arg0, Method arg1) {
                return arg0.getName().compareTo(arg1.getName());
            }
        });
        for (Method met:mets){
            if (met.getName().equalsIgnoreCase("main") || met.getName().startsWith("inline"))
                continue;
            try {
                System.out.println("Test "+met.getName()+" result: "+met.invoke(main));
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        }
    }
    public int testInt_2tc11(){
        int i=0;
        int res=0;
        for (;i<50;i++){
            res+=i;
            res+=inlineIntAndShlXortc1(res, i);
        }
        return res;
    }
    public int testInt_2tc12(){
        int i=0;
        int res=0;
        try {
            for (;i<50;i++){
                res+=i;
                res+=inlineIntAndShlXor(res, i);
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public int testInt_2tc21(){
        int i=0;
        int res=0;
        for (;i<50;i++){
            res+=i;
            res+=inlineIntAndShlXortc2(res, i);
        }
        return res;
    }
    public int testInt_2tc22(){
        int i=0;
        int res=0;
        try {
            for (;i<50;i++){
                res+=i;
                res+=inlineIntAndShlXor(res, i);
                res/=49-i;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public int testInt_2tc31(){
        int i=0;
        int res=0;
        for (;i<50;i++){
            res+=i;
            res+=inlineIntAndShlXortc3(res, i);
        }
        return res;
    }

    public int testInt_2tc32(){
        int i=0;
        int res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e){
            for (;i<50;i++){
                res+=i;
                res+=inlineIntAndShlXor(res, i);
            }
        }
        return res;
    }
    public int testInt_2tc41(){
        int i=0;
        int res=0;
        for (;i<50;i++){
            res+=i;
            res+=inlineIntAndShlXortc3(res, i);
        }
        return res;
    }
    public int testInt_2tc42(){
        int i=0;
        int res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (;i<50;i++){
                res+=i;
                res+=inlineIntAndShlXor(res, i);
            }
        }
        return res;
    }
    public int testInt_2tc52(){
        int i=0;
        int res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e){
            res++;
        }
        for (;i<50;i++){
            res+=i;
            res+=inlineIntAndShlXor(res, i);
        }
        return res;
    }
    public int testInt_2tc62(){
        int i=0;
        int res=0;
        for (;i<50;i++){
            res+=i;
            res+=inlineIntAndShlXor(res, i);
        }
        try {
            throw new Exception("Test");
        } catch (Exception e){
            res++;
        }
        return res;
    }
    public long testLong_3tc1(){
        long i=1;
        long res=1;
        long tmp=10;
        try {
            for (i=0;i<maxIter;i++){
                res*=tmp;
                tmp++;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_3tc2(){
        long i=1;
        long res=1;
        long tmp=10;
        try {
            for (i=0;i<maxIter;i++){
                res*=tmp;
                tmp++;
                tmp/=maxIter-i-1;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_3tc3(){
        long i=1;
        long res=1;
        long tmp=10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (i=0;i<maxIter;i++){
                res*=tmp;
                tmp++;
            }
        }
        return res;
    }
    public long testLong_3tc4(){
        long i=1;
        long res=1;
        long tmp=10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (i=0;i<maxIter;i++){
                res*=tmp;
                tmp++;
            }
        }
        return res;
    }
    public long testLong_3tc5(){
        long i=1;
        long res=1;
        long tmp=10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        for (i=0;i<maxIter;i++){
            res*=tmp;
            tmp++;
        }
        return res;
    }
    public long testLong_3tc6(){
        long i=1;
        long res=1;
        long tmp=10;
        for (i=0;i<maxIter;i++){
            res*=tmp;
            tmp++;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public short testShort_2tc1(){
        short i=0;
        short res=0;
        short tmp=-10;
        try{
            for (;i<50;i++){
                res|=i;
                tmp--;
                res|=tmp;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public short testShort_2tc2(){
        short i=0;
        short res=0;
        short tmp=-10;
        try{
            for (;i<50;i++){
                res|=i;
                tmp--;
                res|=tmp;
                res/=50-i-1;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public short testShort_2tc3(){
        short i=0;
        short res=0;
        short tmp=-10;
        try{
            throw new Exception("Test");
        } catch (Exception e) {
            for (;i<50;i++){
                res|=i;
                tmp--;
                res|=tmp;
            }
        }
        return res;
    }
    public short testShort_2tc4(){
        short i=0;
        short res=0;
        short tmp=-10;
        try{
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (;i<50;i++){
                res|=i;
                tmp--;
                res|=tmp;
            }
        }
        return res;
    }
    public short testShort_2tc5(){
        short i=0;
        short res=0;
        short tmp=-10;
        try{
            throw new Exception("Test");
        } catch (Exception e) {
            res=1;
        }
        for (;i<50;i++){
            res|=i;
            tmp--;
            res|=tmp;
        }
        return res;
    }
    public short testShort_2tc6(){
        short i=0;
        short res=0;
        short tmp=-10;
        for (;i<50;i++){
            res|=i;
            tmp--;
            res|=tmp;
        }
        try{
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public int testFloat_1tc1(){
        int i=1;
        float res=1234567.0f;
        float res1=0.0f;
        try {
            for (;i<5;i++){
                res1+=res%i;
            }
        } catch (Exception e) {
            res++;
        }
        return Float.floatToIntBits(res1);
    }
    public int testFloat_1tc2(){
        int i=1;
        float res=1234567.0f;
        float res1=0.0f;
        try {
            for (;i<5;i++){
                res1+=res%i;
                res+=(100/(5-i-1));
            }
        } catch (Exception e) {
            res++;
        }
        return Float.floatToIntBits(res1);
    }
    public int testFloat_1tc3(){
        int i=1;
        float res=1234567.0f;
        float res1=0.0f;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (;i<5;i++){
                res1+=res%i;
            }
        }
        return Float.floatToIntBits(res1);
    }
    public int testFloat_1tc4(){
        int i=1;
        float res=1234567.0f;
        float res1=0.0f;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (;i<5;i++){
                res1+=res%i;
            }
        }
        return Float.floatToIntBits(res1);
    }
    public int testFloat_1tc5(){
        int i=1;
        float res=1234567.0f;
        float res1=0.0f;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        for (;i<5;i++){
            res1+=res%i;
        }
        return Float.floatToIntBits(res1);
    }
    public int testFloat_1tc6(){
        int i=1;
        float res=1234567.0f;
        float res1=0.0f;
        for (;i<5;i++){
            res1+=res%i;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return Float.floatToIntBits(res1);
    }
    public byte testByte_5tc1(){
        byte i=0;
        byte res=0;
        byte tmp=12;
        try {
            for (;i<5;i++){
                res<<=tmp++<<i;
            }
        }
        catch (Exception e) {
            res++;
        }
        return res;
    }
    public byte testByte_5tc2(){
        byte i=0;
        byte res=0;
        byte tmp=12;
        try {
            for (;i<5;i++){
                res<<=tmp++<<i;
                res/=5-i-1;
            }
        }
        catch (Exception e) {
            res++;
        }
        return res;
    }
    public byte testByte_5tc3(){
        byte i=0;
        byte res=0;
        byte tmp=12;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (;i<5;i++){
                res<<=tmp++<<i;
            }
        }
        return res;
    }
    public byte testByte_5tc4(){
        byte i=0;
        byte res=0;
        byte tmp=12;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (;i<5;i++){
                res<<=tmp++<<i;
            }
        }
        return res;
    }
    public byte testByte_5tc5(){
        byte i=0;
        byte res=0;
        byte tmp=12;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res=1;
        }
        for (;i<5;i++){
            res<<=tmp++<<i;
        }
        return res;
    }
    public byte testByte_5tc6(){
        byte i=0;
        byte res=0;
        byte tmp=12;
        for (;i<5;i++){
            res<<=tmp++<<i;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public short testShort_7tc1(){
        short i=0;
        short res=0;
        short tmp=-10;
        try {
            for (i=0;i<maxIter;i++){
                res>>=tmp>>i;
                tmp++;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public short testShort_7tc2(){
        short i=0;
        short res=0;
        short tmp=-10;
        try {
            for (i=0;i<maxIter;i++){
                res>>=tmp>>i;
                tmp++;
                tmp/=maxIter-i-1;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public short testShort_7tc3(){
        short i=0;
        short res=0;
        short tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (i=0;i<maxIter;i++){
                res>>=tmp>>i;
                tmp++;
            }
        }
        return res;
    }
    public short testShort_7tc4(){
        short i=0;
        short res=0;
        short tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (i=0;i<maxIter;i++){
                res>>=tmp>>i;
                tmp++;
            }
        }
        return res;
    }
    public short testShort_7tc5(){
        short i=0;
        short res=0;
        short tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res=1;
        }
        for (i=0;i<maxIter;i++){
            res>>=tmp>>i;
            tmp++;
        }
        return res;
    }
    public short testShort_7tc6(){
        short i=0;
        short res=0;
        short tmp=-10;
        for (i=0;i<maxIter;i++){
            res>>=tmp>>i;
            tmp++;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_6tc1(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            for (;i<50;i++){
                res=tmp-i;
                tmp++;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_6tc2(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            for (;i<50;i++){
                res=tmp-i;
                tmp++;
                res/=49-i;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_6tc3(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (;i<50;i++){
                res=tmp-i;
                tmp++;
            }
        }
        return res;
    }
    public long testLong_6tc4(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (;i<50;i++){
                res=tmp-i;
                tmp++;
            }
        }
        return res;
    }
    public long testLong_6tc5(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        for (;i<50;i++){
            res=tmp-i;
            tmp++;
        }
        return res;
    }
    public long testLong_6tc6(){
        long i=0;
        long res=0;
        long tmp=-10;
        for (;i<50;i++){
            res=tmp-i;
            tmp++;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_7tc1(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            for (i=0;i<maxIter;i++){
                res>>>=tmp>>>i;
                tmp++;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_7tc2(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            for (i=0;i<maxIter;i++){
                res>>>=tmp>>>i;
                tmp++;
                res/=maxIter-i-1;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public long testLong_7tc3(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (i=0;i<maxIter;i++){
                res>>>=tmp>>>i;
                tmp++;
            }
        }
        return res;
    }
    public long testLong_7tc4(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (i=0;i<maxIter;i++){
                res>>>=tmp>>>i;
                tmp++;
            }
        }
        return res;
    }
    public long testLong_7tc5(){
        long i=0;
        long res=0;
        long tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        for (i=0;i<maxIter;i++){
            res>>>=tmp>>>i;
            tmp++;
        }
        return res;
    }
    public long testLong_7tc6(){
        long i=0;
        long res=0;
        long tmp=-10;
        for (i=0;i<maxIter;i++){
            res>>>=tmp>>>i;
            tmp++;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public byte testByte_2tc1(){
        byte i=0;
        byte res=0;
        byte tmp=-10;
        try {
            for (;i<50;i++){
                res^=i;
                tmp--;
                res^=tmp;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public byte testByte_2tc2(){
        byte i=0;
        byte res=0;
        byte tmp=-10;
        try {
            for (;i<50;i++){
                res^=i;
                tmp--;
                res^=tmp;
                res/=49-i;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public byte testByte_2tc3(){
        byte i=0;
        byte res=0;
        byte tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (;i<50;i++){
                res^=i;
                tmp--;
                res^=tmp;
            }
        }
        return res;
    }
    public byte testByte_2tc4(){
        byte i=0;
        byte res=0;
        byte tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        } finally {
            for (;i<50;i++){
                res^=i;
                tmp--;
                res^=tmp;
            }
        }
        return res;
    }
    public byte testByte_2tc5(){
        byte i=0;
        byte res=0;
        byte tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res=1;
        }
        for (;i<50;i++){
            res^=i;
            tmp--;
            res^=tmp;
        }
        return res;
    }
    public byte testByte_2tc6(){
        byte i=0;
        byte res=0;
        byte tmp=-10;
        for (;i<50;i++){
            res^=i;
            tmp--;
            res^=tmp;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public short testShort_8tc1(){
        short i=1;
        short res=12;
        short res1=10000;
        short tmp=-10;
        try {
            for (i=tmp;i<res;i++){
                res1/=10;
            }
        } catch (NullPointerException e) {
            res1++;
        }
        return res1;
    }
    public short testShort_8tc2(){
        short i=1;
        short res=12;
        short res1=10000;
        short tmp=-10;
        try {
            for (i=tmp;i<res;i++){
                res1/=res-tmp;
                res1/=i-res+1;
            }
        } catch (Exception e) {
            res1++;
        }
        return res1;
    }
    public short testShort_8tc3(){
        short i=1;
        short res=12;
        short res1=10000;
        short tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (i=tmp;i<res;i++){
                res1/=res-tmp;
            }
        }
        return res1;
    }
    public short testShort_8tc4(){
        short i=1;
        short res=12;
        short res1=10000;
        short tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res1++;
        } finally {
            for (i=tmp;i<res;i++){
                res1/=res-tmp;
            }
        }
        return res1;
    }
    public short testShort_8tc5(){
        short i=1;
        short res=12;
        short res1=10000;
        short tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res1=10001;
        }
        for (i=tmp;i<res;i++){
            res1/=res-tmp;
        }
        return res1;
    }
    public short testShort_8tc6(){
        short i=1;
        short res=12;
        short res1=10000;
        short tmp=-10;
        for (i=tmp;i<res;i++){
            res1/=res-tmp;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res1++;
        }
        return res1;
    }
    public int testInt_3tc1(){
        int i=0;
        int res=0;
        int tmp=-10;
        try {
            for (i=0;i<maxIter;i++){
                res&=tmp;
                tmp++;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public int testInt_3tc2(){
        int i=0;
        int res=0;
        int tmp=-10;
        try {
            for (i=0;i<maxIter;i++){
                res&=tmp;
                tmp++;
                res/=maxIter-i-1;
            }
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public int testInt_3tc3(){
        int i=0;
        int res=0;
        int tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (i=0;i<maxIter;i++){
                res&=tmp;
                tmp++;
            }
        }
        return res;
    }
    public int testInt_3tc4(){
        int i=0;
        int res=0;
        int tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        finally {
            for (i=0;i<maxIter;i++){
                res&=tmp;
                tmp++;
            }
        }
        return res;
    }
    public int testInt_3tc5(){
        int i=0;
        int res=0;
        int tmp=-10;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        for (i=0;i<maxIter;i++){
            res&=tmp;
            tmp++;
        }
        return res;
    }
    public int testInt_3tc6(){
        int i=0;
        int res=0;
        int tmp=-10;
        for (i=0;i<maxIter;i++){
            res&=tmp;
            tmp++;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res++;
        }
        return res;
    }
    public byte testByte_1tc01(){
        byte i=0;
        byte res=0;
        try {
            for (;i<5;i++){
                res+=i;
            }
        } catch (Exception e) {
            res+=1;
        }
        return res;
    }
    public byte testByte_1tc02(){
        byte i=0;
        byte res=0;
        try {
            for (;i<5;i++){
                res+=i;
                res/=4-i;
            }
        } catch (Exception e) {
            res+=1;
        }
        return res;
    }
    public byte testByte_1tc03(){
        byte i=0;
        byte res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (;i<5;i++){
                res+=i;
            }
        }
        return res;
    }
    public byte testByte_1tc04(){
        byte i=0;
        byte res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res-=1;
        } finally {
            for (;i<5;i++){
                res+=i;
            }
        }
        return res;
    }
    public byte testByte_1tc05(){
        byte i=0;
        byte res=0;
        try {
            for (;i<5;i++){
                res+=i;
                res+=inlined(res);
            }
        } catch (Exception e) {
            res+=1;
        }
        return res;
    }
    public byte testByte_1tc06(){
        byte i=0;
        byte res=0;
        try {
            for (;i<5;i++){
                res+=i;
                res/=4-i;
                res+=inlined(res);
            }
        } catch (Exception e) {
            res+=1;
        }
        return res;
    }
    public byte testByte_1tc07(){
        byte i=0;
        byte res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            for (;i<5;i++){
                res+=i;
                res+=inlined(res);
            }
        }
        return res;
    }
    public byte testByte_1tc08(){
        byte i=0;
        byte res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res-=1;
        } finally {
            for (;i<5;i++){
                res+=i;
                res+=inlined(res);
            }
        }
        return res;
    }
    public byte testByte_1tc09(){
        byte i=0;
        byte res=0;
        for (;i<5;i++){
            try {
                throw new Exception("Test");
            } catch (Exception e) {
                res-=1;
            } finally {
                res++;
            }
            res+=i;
        }
        return res;
    }
    public byte testByte_1tc10(){
        byte i=0;
        byte res=0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res=1;
        }
        for (;i<5;i++){
            res+=i;
        }
        return res;
    }
    public byte testByte_1tc11(){
        byte i=0;
        byte res=0;
        for (;i<5;i++){
            res+=i;
        }
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            res+=1;
        }
        return res;
    }
}