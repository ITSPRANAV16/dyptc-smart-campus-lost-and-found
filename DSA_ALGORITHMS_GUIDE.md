# 📚 Dr. D. Y. Patil Technical Campus (DYPTC)
# Smart Campus Lost & Found System — Complete DSA & Sorting Documentation
> **विषय:** Data Structures & Algorithms (DSA) Course Project Guide  
> **अभ्यासक्रम:** Second Year B.Tech Computer Engineering (Semester 03 - SPPU)  
> **प्रकल्प:** Smart Campus Lost & Found Matching System  
> **तंत्रज्ञान:** C++17, Crow Framework, SQLite3, Pure HTML5/CSS3  

---

## 📑 अनुक्रमणिका (Table of Contents)
1. [प्रकल्प विहंगावलोकन (Project Overview)](#1-प्रकल्प-विहंगावलोकन-project-overview)
2. [सर्व Sorting Algorithms: काय, कुठे आणि का वापरले?](#2-सर्व-sorting-algorithms-काय-कुठे-आणि-का-वापरले)
   - [२.१ Merge Sort (गॅरंटीड O(N log N))](#२१-merge-sort-divide--conquer)
   - [२.२ Insertion Sort (अ‍ॅडॉप्टिव्ह O(N²))](#२२-insertion-sort-adaptive-shift)
   - [२.३ Bubble Sort (अर्ली एक्झिट O(N²))](#२३-bubble-sort-early-termination)
   - [२.४ Sorting Algorithms तुलनात्मक विश्लेषण (Comparison Table)](#२४-sorting-algorithms-तुलनात्मक-विश्लेषण)
3. [Searching Algorithms: Linear Search vs Binary Search](#3-searching-algorithms-linear-search-vs-binary-search)
   - [३.१ Linear Search (O(N) Multi-Criteria Scan)](#३१-linear-search-बहुआयामी-शोध)
   - [३.२ Binary Search (O(log N) Divide & Conquer)](#३२-binary-search-विभाजन-आणि-विजय)
4. [Data Structures (डेटा स्ट्रक्चर्स): प्रकार व कारणे](#4-data-structures-डेटा-स्ट्रक्चर्स-प्रकार-व-कारणे)
   - [४.१ Priority Queue / Max-Heap (ऑटो-मॅच रँकिंग)](#४१-priority-queue--max-heap-auto-matches-ranking)
   - [४.२ Dynamic Array (`std::vector<Item>`)](#४२-dynamic-array-stdvector)
   - [४.३ Hash Table Indexing (`std::unordered_map` & `std::unordered_set`)](#४३-hash-table-indexing-stdunordered_map)
   - [४.४ Custom Functor / Operator Overloading (`MatchComparator`)](#४४-custom-functor-matchcomparator)
5. [Specialized Algorithms (विशेष अल्गोरिदम्स)](#5-specialized-algorithms-विशेष-अल्गोरिदम्स)
   - [५.१ Dynamic Programming: Levenshtein Distance (अचूक स्पेलिंग तपासणी)](#५१-dynamic-programming-levenshtein-distance)
   - [५.२ Set Theory: Jaccard Token Similarity (शब्दांचा क्रम बदलला तरी मॅच)](#५२-set-theory-jaccard-token-similarity)
   - [५.३ Weighted 6-Factor Multi-Attribute Scoring Engine](#५३-weighted-6-factor-multi-attribute-scoring-engine)
   - [५.४ Spatial Frequency Matrix (Campus Heatmap & Analytics)](#५४-spatial-frequency-matrix)
   - [५.५ Cryptographic Claim Token Generator](#५५-cryptographic-claim-token-generator)
6. [कोड ट्रेसेबिलिटी तक्ता (Code Traceability Table)](#6-कोड-ट्रेसेबिलिटी-तक्ता-code-traceability-table)
7. [मॅडम विचारू शकतील असे टॉप १० Viva Questions & Answers](#7-मॅडम-विचारू-शकतील-असे-टॉप-१०-viva-questions--answers)

---

## 1. प्रकल्प विहंगावलोकन (Project Overview)

आपल्या कॉलेज कॅम्पसमध्ये (DYPTC Talegaon-Varale) विद्यार्थ्यांचे लॅपटॉप, कॅल्क्युलेटर, ओळखपत्र (ID Card), किल्ल्या, बॅग्स हरवल्यास किंवा सापडल्यास मॅन्युअल रजिस्टरमध्ये नोंद करावी लागत असे. त्यामुळे वस्तू परत मिळण्यात खूप उशीर व्हायचा आणि फसवणूक होण्याची भीती असायची.

यावर उपाय म्हणून आपण **C++17** आणि **Data Structures & Algorithms (DSA)** चा वापर करून एक संपूर्ण ऑटोमेटेड वेब पोर्टल तयार केले आहे. या सिस्टीममधील सर्व बिझनेस लॉजिक, सॉर्टिंग, सर्चिंग, स्ट्रिंग मॅचिंग आणि रँकिंग हे कोणत्याही बाह्य लायब्ररी किंवा पायथन/जावास्क्रिप्टवर अवलंबून नसून **शुद्ध C++ अल्गोरिदम्सद्वारे** रन होते.

---

## 2. सर्व Sorting Algorithms: काय, कुठे आणि का वापरले?

प्रकल्पामध्ये **तीन प्रमुख सॉर्टिंग अल्गोरिदम्स** प्रत्यक्ष कोड केले आहेत आणि ते युझर/ॲडमिनला `items.html` (`/items`) पेजवर थेट रन करून तपासता येतात:

```
                  ┌─────────────────────────────────────────┐
                  │       Master Sorting Demonstrator       │
                  └────────────────────┬────────────────────┘
                                       │
        ┌──────────────────────────────┼──────────────────────────────┐
        ▼                              ▼                              ▼
  [ Merge Sort ]              [ Insertion Sort ]              [ Bubble Sort ]
  O(N log N) Stable            O(N²) Adaptive                  O(N²) Teaching
  Production Default           Near-Sorted Data                Early Exit Flag
```

---

### २.१ Merge Sort (Divide & Conquer)
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L295-L350) (`dsa::mergeSort`, `dsa::merge`)
* **Time Complexity:** 
  - Best Case: $O(N \log N)$
  - Average Case: $O(N \log N)$
  - Worst Case: $O(N \log N)$
* **Space Complexity:** $O(N)$ (Auxiliary vector for merging)
* **Stability:** Stable (समान तारीख किंवा नाव असणाऱ्या आयटम्सचा मूळ क्रम कायम राहतो).

#### 🤔 का वापरले? (Why Used?):
1. **गॅरंटीड परफॉर्मन्स:** हजारो वस्तूंची नोंद झाली तरी Merge Sort कधीही स्लो होत नाही ($O(N \log N)$ गॅरंटीड).
2. **Stable Sorting:** जर दोन विद्यार्थ्यांच्या वस्तूंची तारीख एकच असेल, तर आधी नोंदवलेली वस्तू आधीच दिसते (क्रॉस-ऑर्डर होत नाही).
3. **Binary Search ची पूर्वतयारी:** Binary Search चालवण्यासाठी डेटा अगोदर सॉर्टेड असावा लागतो. Binary Search कॉल करण्यापूर्वी आपण Merge Sort ने संपूर्ण अरे सॉर्ट करतो.

---

### २.२ Insertion Sort (Adaptive Shift)
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L246-L290) (`dsa::insertionSort`)
* **Time Complexity:** 
  - Best Case: $O(N)$ (जेव्हा डेटा आधीच जवळपास सॉर्ट केलेला असतो)
  - Average Case: $O(N^2)$
  - Worst Case: $O(N^2)$
* **Space Complexity:** $O(1)$ (In-place)
* **Stability:** Stable.

#### 🤔 का वापरले? (Why Used?):
1. **जवळपास सॉर्टेड डेटासाठी सर्वात वेगवान:** कॅम्पसमध्ये रोज नवीन १-२ आयटम्सचीच नोंद होते. अशा वेळी संपूर्ण डेटा पुन्हा नव्याने सॉर्ट करण्यापेक्षा Insertion Sort फक्त नवीन घटकाला योग्य जागी सरकवतो, जे केवळ $O(N)$ वेळेत होते.
2. **शून्य जास्तीची मेमरी ($O(1)$):** Merge Sort प्रमाणे याला वेगळा अरे लागत नाही.
3. **शैक्षणिक प्रात्यक्षिक (Demonstration):** वर्गात शिकवलेला Insertion Sort प्रत्यक्ष वेब ॲपमध्ये कसा चालतो हे मॅडमना दाखवण्यासाठी.

---

### २.३ Bubble Sort (Early Termination)
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L209-L238) (`dsa::bubbleSort`)
* **Time Complexity:** 
  - Best Case: $O(N)$ (Optimized `swapped` flag मुळे)
  - Average Case: $O(N^2)$
  - Worst Case: $O(N^2)$
* **Space Complexity:** $O(1)$ (In-place)
* **Stability:** Stable.

#### 🤔 का वापरले? (Why Used?):
1. **DSA Lab Comparison Metric:** $O(N^2)$ अल्गोरिदम मोठ्या डेटामध्ये कसा वेळ घेतो आणि $O(N \log N)$ चा Merge Sort कसा सेकंदात काम करतो, यामधील प्रत्यक्ष रनटाईम फरक ($\mu s$) स्क्रीनवर दाखवण्यासाठी.
2. **Early Exit Optimization:** जर एका फेरीत एकही स्वॅप झाला नाही (`if (!swapped) break;`), तर लूप लगेच थांबतो, ज्यामुळे अनावश्यक फेऱ्या वाचतात.

---

### २.४ Sorting Algorithms तुलनात्मक विश्लेषण

| Sorting Algorithm | Best Case | Average Case | Worst Case | Space Complexity | Stable? | प्रत्यक्ष कॅम्पस उपयोग |
|---|:---:|:---:|:---:|:---:|:---:|---|
| **Merge Sort** | $O(N \log N)$ | $O(N \log N)$ | $O(N \log N)$ | $O(N)$ | होय | कॅटलॉगचा मुख्य डीफॉल्ट सॉर्ट + Binary Search ची पूर्वतयारी |
| **Insertion Sort** | $O(N)$ | $O(N^2)$ | $O(N^2)$ | $O(1)$ | होय | रोजच्या १-२ नवीन नोंदींसाठी जलद इन-प्लेस अ‍ॅडजस्टमेंट |
| **Bubble Sort** | $O(N)$ | $O(N^2)$ | $O(N^2)$ | $O(1)$ | होय | लॅब इव्हॅल्यूएशनमध्ये रनटाईम व स्वॅप्स दाखवण्यासाठी |

---

## 3. Searching Algorithms: Linear Search vs Binary Search

प्रकल्पामध्ये **दोन स्वतंत्र शोध अल्गोरिदम्स** असून ॲडमिन `search.html` (`/search`) वर दोघांची तुलना करू शकतो:

```
┌────────────────────────────────────────────────────────────────────────┐
│                   Search Campus Belongings Engine                      │
├───────────────────────────────────┬────────────────────────────────────┤
│           Linear Search           │           Binary Search            │
│       O(N) Sequential Scan        │      O(log N) Divide & Conquer     │
│   • Unsorted डेटावर चालतो         │   • डेटा आधी Sorted असावा लागतो    │
│   • Multi-criteria (नाव+जागा+रंग) │   • नाव (Item Name) वर अचूक शोध    │
│   • सर्व फील्ड्स स्कॅन करतो       │   • दर पायरीला शोधक्षेत्र अर्धे होते│
└───────────────────────────────────┴────────────────────────────────────┘
```

### ३.१ Linear Search (बहुआयामी शोध)
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L365-L415) (`dsa::linearSearch`)
* **Time Complexity:** $O(N)$
* **Space Complexity:** $O(1)$

#### 🤔 का वापरले?
विद्यार्थी जेव्हा वस्तू शोधतो, तेव्हा त्याला फक्त नाव नाही तर **कॅटेगरी, हरवलेले ठिकाण (Location), रंग, आणि स्टेटस** या सर्व निकषांवर एकाच वेळी फिल्टर करायचे असते. डेटा कोणत्याही क्रमाने असला तरी Linear Search प्रत्येक घटकाची तपासणी करून १००% अचूक रिझल्ट देतो.

---

### ३.२ Binary Search (विभाजन आणि विजय)
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L420-L480) (`dsa::binarySearchByName`)
* **Time Complexity:** $O(\log N)$
* **Space Complexity:** $O(1)$

#### 🤔 का वापरले?
जर हजारो वस्तूंची नोंद असेल, तर एकामागून एक शोधत बसण्यापेक्षा अरेच्या मध्यावर (`mid = (low + high) / 2`) तपासणी केली जाते. यामुळे प्रत्येक पायरीला शोधक्षेत्र **५०% ने कमी होते**. 
- उदा. १०२४ वस्तूंमध्ये Linear Search ला १०२४ तपासण्या लागू शकतात, पण Binary Search **फक्त १० तपासण्यांमध्ये ($2^{10} = 1024$)** वस्तू शोधून काढतो!

---

## 4. Data Structures (डेटा स्ट्रक्चर्स): प्रकार व कारणे

### ४.१ Priority Queue / Max-Heap (Auto-Matches Ranking)
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L510-L580) (`dsa::findRankedMatches`)
* **C++ STL:** `std::priority_queue<MatchResult, std::vector<MatchResult>, MatchComparator>`
* **Time Complexity:** 
  - Insert (Push): $O(\log K)$
  - Extract Max (Pop): $O(\log K)$
  - Peek (Top): $O(1)$

#### 🤔 का वापरले? (Most Important Viva Question!)
मॅडम विचारतील: *"सर्व जोड्या सॉर्ट करण्यासाठी तुम्ही Quicksort का नाही वापरला? Priority Queue का वापरली?"*
* **उत्तर:**
  - जर $100$ हरवलेल्या वस्तू आणि $100$ सापडलेल्या वस्तू असतील, तर एकूण $100 \times 100 = 10,000$ जोड्या तयार होतात.
  - सर्व १०,००० जोड्या Quicksort ने सॉर्ट केल्यास $O(M \cdot N \log(M \cdot N))$ वेळ लागेल आणि खूप रॅम खर्च होईल.
  - पण विद्यार्थ्याला **फक्त Top 5 किंवा Top 10 सर्वोत्तम जुळणाऱ्या वस्तूच** हव्या असतात!
  - **Max-Heap (Priority Queue)** वापरल्यामुळे ज्या जोडीचा मॅच स्कोअर सर्वाधिक आहे, ती थेट $O(1)$ वेळेत वर येते आणि अवघ्या $O(\log K)$ वेळेत बाहेर काढता येते!

```
                  ┌───────────────────────┐
                  │ Max-Heap (Top Match)  │
                  │ Score: 95% (Earbuds)  │
                  └───────────┬───────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
    ┌──────────────────┐            ┌──────────────────┐
    │ Score: 82% (Bag) │            │ Score: 78% (Key) │
    └──────────────────┘            └──────────────────┘
```

---

### ४.२ Dynamic Array (`std::vector<Item>`)
* **फाईल:** [src/models.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/models.hpp#L15-L35)
* **Time Complexity:** Random Access $O(1)$, Append $O(1)$ amortized.

#### 🤔 का वापरले?
कॅम्पसमध्ये वस्तूंची संख्या आधीच ठरलेली नसते. Static Array (`Item arr[100]`) वापरल्यास मेमरी वाया जाते किंवा मेमरी संपते. `std::vector` आपोआप आवश्यकतेनुसार आकार वाढवतो (Dynamic Resizing) आणि Cache Locality मुळे CPU मध्ये अत्यंत जलद काम करतो.

---

### ४.३ Hash Table Indexing (`std::unordered_map`)
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L150-L162) (`dsa::buildCategoryIndex`)
* **Time Complexity:** Average Lookup $O(1)$, Worst $O(N)$

#### 🤔 का वापरले?
1. **Category Inverted Index:** कॅटेगरीनुसार (उदा. `"Electronics" -> [1, 4, 7]`) वस्तूंचे आयडी हॅश टेबलमध्ये साठवले जातात. जेव्हा कोणी इलेक्ट्रॉनिक्स सिलेक्ट करतो, तेव्हा पूर्ण डेटाबेस न शोधता $O(1)$ मध्ये थेट यादी मिळते.
2. **Bilingual Translation Engine:** [html_renderer.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/html_renderer.hpp#L173-L195) मध्ये इंग्रजी आणि मराठी शब्दांचे भाषांतर $O(1)$ वेळेत करण्यासाठी हॅश मॅप वापरला आहे.

---

### ४.४ Custom Functor (`MatchComparator`)
* **फाईल:** [src/models.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/models.hpp#L75-L81)
```cpp
struct MatchComparator {
    bool operator()(const MatchResult& a, const MatchResult& b) const {
        return a.breakdown.total_score < b.breakdown.total_score;
    }
};
```
#### 🤔 का वापरले?
Priority Queue ला दोन वस्तूंमधील कोणता घटक मोठा मानायचा हे ठरवण्यासाठी C++ मध्ये Functor (Function Object) लागतो. या ऑपरेटर ओव्हरलोडिंगमुळे Heap मध्ये सर्वात जास्त गुण असलेली वस्तू सर्वात वर राहते.

---

## 5. Specialized Algorithms (विशेष अल्गोरिदम्स)

### ५.१ Dynamic Programming: Levenshtein Distance
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L67-L93) (`dsa::levenshteinDistance`)
* **Time Complexity:** $O(M \times N)$
* **Space Complexity:** $O(M \times N)$ DP Matrix

#### 🤔 का वापरले?
विद्यार्थी वस्तूचे स्पेलिंग टाकताना चुका करू शकतात. 
- उदा. हरवलेल्या वस्तूत **"AirPods"** लिहिले आणि शोधणाऱ्याने **"Earbuds"** किंवा **"Airpod"** लिहिले.
- साध्या `==` ने हे कधीच मॅच होणार नाही.
- Levenshtein DP अल्गोरिदम दोन शब्दांमधील Minimum Edits (Insertion, Deletion, Substitution) मोजतो.
$$\text{Cost Matrix}: DP[i][j] = \min(DP[i-1][j]+1, DP[i][j-1]+1, DP[i-1][j-1] + \text{cost})$$
त्यामुळे ९०% स्पेलिंग साम्य ओळखले जाते.

---

### ५.२ Set Theory: Jaccard Token Similarity
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L114-L135) (`dsa::jaccardSimilarity`)
* **Formula:**
$$J(A, B) = \frac{|A \cap B|}{|A \cup B|}$$

#### 🤔 का वापरले?
जेव्हा शब्दांचा क्रम बदलतो:
- युझर १: `"HP Silver Laptop 15s"`
- युझर २: `"Silver Laptop HP"`
या दोघांमध्ये शब्दांचा क्रम उलटा असला तरी Tokens चे Intersection ($|A \cap B| = 3$) आणि Union समान असल्याने १००% मॅच मिळतो!

---

### ५.३ Weighted 6-Factor Multi-Attribute Scoring Engine
* **फाईल:** [src/dsa_engine.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L510-L570)
* वस्तूंची अचूक पडताळणी करण्यासाठी आपण खालील ६ घटकांचे वजन (Weightage) ठरवले आहे:

| घटक (Attribute) | Weightage | अल्गोरिदम |
|---|:---:|---|
| **१. Category (प्रकार)** | **२५%** | Exact Hash Match ($O(1)$) |
| **२. Name (वस्तूचे नाव)** | **२५%** | Hybrid: ६०% Jaccard + ४०% Levenshtein DP |
| **३. Location (जागा)** | **१५%** | Token Intersect + Exact Substring |
| **४. Colour (रंग)** | **१५%** | Levenshtein Tolerance |
| **५. Description Keywords** | **१०%** | Token Jaccard Set Overlap |
| **६. Date Proximity (तारीख)** | **१०%** | दिवसांमधील अंतर (Exponential Decay) |
| **एकूण (Total)** | **१००%** | **Priority Queue Max-Heap मध्ये रँक केले जाते** |

---

### ५.४ Spatial Frequency Matrix (Heatmap & Analytics)
* **फाईल:** [main.cpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/main.cpp#L1714-L1765) (`/heatmap`, `/analytics`)
* कॅम्पसमधील ६ प्रमुख झोन्स (Central Library, Canteen, AI&DS Labs, Architecture Studio, Parking, Hostels) यांना 2D Matrix मध्ये मॅप करून कोणत्या भागात जास्त वस्तू हरवतात याचा Frequency Heatmap तयार केला जातो.

---

### ५.५ Cryptographic Claim Token Generator
* **फाईल:** [src/database.hpp](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/database.hpp#L45-L60)
* विद्यार्थ्यांची वैयक्तिक माहिती सुरक्षित ठेवण्यासाठी आणि बनावट क्लेम्स रोखण्यासाठी प्रत्येक नोंदीला **`DYPTC-LST-01-9F2A`** असा युनिक सिक्युरिटी हॅश टोकन दिला जातो.

---

## 6. कोड ट्रेसेबिलिटी तक्ता (Code Traceability Table)

मॅडमना थेट फाईल आणि ओळ क्रमांक दाखवण्यासाठी हा संदर्भ तक्ता वापरा:

| DSA Concept | फाईलचे नाव | फाईल लिंक | फंक्शन्स / स्ट्रक्चर्स |
|---|---|---|---|
| **Merge Sort** | `src/dsa_engine.hpp` | [ओळ 295 ते 350](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L295-L350) | `dsa::mergeSort`, `dsa::merge` |
| **Insertion Sort** | `src/dsa_engine.hpp` | [ओळ 246 ते 290](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L246-L290) | `dsa::insertionSort` |
| **Bubble Sort** | `src/dsa_engine.hpp` | [ओळ 209 ते 238](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L209-L238) | `dsa::bubbleSort` |
| **Linear Search** | `src/dsa_engine.hpp` | [ओळ 365 ते 415](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L365-L415) | `dsa::linearSearch` |
| **Binary Search** | `src/dsa_engine.hpp` | [ओळ 420 ते 480](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L420-L480) | `dsa::binarySearchByName` |
| **Priority Queue (Heap)** | `src/dsa_engine.hpp` | [ओळ 510 ते 580](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L510-L580) | `std::priority_queue`, `findRankedMatches` |
| **Levenshtein DP** | `src/dsa_engine.hpp` | [ओळ 67 ते 106](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L67-L106) | `dsa::levenshteinDistance`, `levenshteinRatio` |
| **Jaccard Similarity** | `src/dsa_engine.hpp` | [ओळ 114 ते 145](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L114-L145) | `dsa::jaccardSimilarity`, `tokenize` |
| **Category Hash Map** | `src/dsa_engine.hpp` | [ओळ 150 ते 162](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/dsa_engine.hpp#L150-L162) | `dsa::buildCategoryIndex` |
| **Functor Comparator** | `src/models.hpp` | [ओळ 75 ते 81](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/models.hpp#L75-L81) | `struct MatchComparator` |
| **Data Models (Item)** | `src/models.hpp` | [ओळ 15 ते 55](file:///c:/Users/admin/OneDrive/Desktop/BTECH/SEM_03/08_DSA_LAB/COURSE_PROJECT/src/models.hpp#L15-L55) | `struct Item`, `struct MatchBreakdown` |

---

## 7. मॅडम विचारू शकतील असे टॉप १० Viva Questions & Answers

### Q1. Why did you use Merge Sort as the primary catalog sorter?
> **Answer:** Merge Sort guarantees $O(N \log N)$ worst-case time complexity and is a **stable** sorting algorithm. In a campus lost-and-found system, if two items have identical dates, stability ensures their original registration sequence is preserved without jittering.

### Q2. What is the prerequisite for Binary Search, and how is it satisfied in your code?
> **Answer:** Binary Search strictly requires the underlying array to be pre-sorted. In our `search.html` controller, before invoking `dsa::binarySearchByName()`, we sort the item collection alphabetically using `dsa::mergeSort()` on the `name` field, ensuring correct $O(\log N)$ divide-and-conquer execution.

### Q3. Why use a Priority Queue (Max-Heap) for auto-matches instead of standard QuickSort?
> **Answer:** In auto-matching, pairing $N$ lost items with $M$ found items produces $N \times M$ candidate pairs. Sorting all pairs with QuickSort takes $O(K \log K)$ where $K = N \times M$. But users only care about the top 5 highest-confidence matches. A Max-Heap dynamically maintains the highest score at the root in $O(1)$ and extracts the top elements in $O(K \log 5)$ time, saving substantial CPU cycles and memory.

### Q4. How does your system handle typos or spelling mistakes in lost item names?
> **Answer:** We implemented the **Levenshtein Distance** algorithm using Dynamic Programming in $O(M \times N)$ time. It calculates the minimum edit operations (insertions, deletions, substitutions) required to transform one string into another. We normalize this into a similarity ratio between $0.0$ and $1.0$.

### Q5. What happens if a student writes "Silver HP Laptop" and the finder wrote "Laptop HP Silver"?
> **Answer:** Simple substring comparison or Levenshtein distance alone performs poorly on word permutations. Therefore, we use **Jaccard Token Similarity** ($J = \frac{|A \cap B|}{|A \cup B|}$), which splits strings into sets of tokens. Since the token sets are identical regardless of word order, Jaccard produces a $1.0$ ($100\%$) match. Our hybrid engine blends $60\%$ Jaccard with $40\%$ Levenshtein.

### Q6. What is the Space Complexity of your Merge Sort vs Bubble Sort?
> **Answer:** Merge Sort requires $O(N)$ auxiliary memory because the merging step needs temporary buffers to store merged halves. Bubble Sort and Insertion Sort operate in-place with $O(1)$ auxiliary space.

### Q7. How does the Bubble Sort early-termination optimization work?
> **Answer:** In standard Bubble Sort, $N-1$ passes are always executed ($O(N^2)$). We introduced a boolean `swapped` flag. If a full pass completes without a single adjacent swap, the array is already sorted, and the algorithm terminates immediately, yielding $O(N)$ best-case time.

### Q8. Where is the Hash Table used in the project?
> **Answer:** We utilize `std::unordered_map` in two places:
> 1. `dsa::buildCategoryIndex()` creates an inverted index mapping each category to a vector of item IDs for $O(1)$ subset retrieval.
> 2. `html::tr()` implements an in-memory dictionary for real-time English-to-Marathi translations.

### Q9. How do you measure the microsecond execution time shown on the screen?
> **Answer:** We use C++ `<chrono>` high-resolution clock (`std::chrono::high_resolution_clock::now()`) before and after the algorithm loop. We record `metrics.duration_microseconds`, `metrics.comparisons`, and `metrics.swaps_or_shifts`, passing these directly into HTML placeholders.

### Q10. How does the system prevent student fraud while preserving DSA transparency?
> **Answer:** We implemented **Zero-Knowledge Blind Custody (RBAC)**. Students can only report lost items and query their private status via unique claim tokens (`/track`), preventing fraudsters from viewing custody items. However, the authenticated Admin (`/admin`) retains full access to the Master Catalog and DSA benchmarking tools (`/search` and `/items`) for academic evaluation.

---
*दस्तऐवज तयार केले: Dr. D. Y. Patil Technical Campus (DYPTC) Pune &bull; C++17 DSA Course Project*
