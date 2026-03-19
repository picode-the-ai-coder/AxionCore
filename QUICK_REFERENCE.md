# ⚡ AXION IDE - QUICK REFERENCE GUIDE

## **Everything You Need to Know in One Page**

---

## 🎯 IDE TABS (Left to Right)

```
[Editor] [Files] [Test] [Contracts] [Deploy] [State] [Events] [Docs]
   ↓        ↓       ↓         ↓         ↓       ↓       ↓       ↓
 Write   Manage   Test     View      Deploy   Check   Track   Read
 Code    Files   Code    Contracts  Smart    State   Events  Help
                                   Contracts
```

---

## 📝 QUICK COMMANDS

| Task | Location | Action |
|------|----------|--------|
| **Create File** | [Files] | Type name → Click Create |
| **Edit Code** | [Editor] | Paste/write Python code |
| **Save File** | [Editor] | Click Save button |
| **Test Code** | [Test] | Click "Run Contract" |
| **Analyze Code** | [Test] | Click "Analyze Code" |
| **Run Function** | [Test] | Type function name → Click Run |
| **Deploy** | [Deploy] | Click Deploy button |
| **Check State** | [State] | View contract variables |
| **View Events** | [Events] | Monitor execution events |

---

## 🔄 MAIN WORKFLOW (5 Steps)

```
Step 1: CREATE          Step 2: EDIT           Step 3: TEST
┌─────────────────┐  ┌─────────────────┐   ┌──────────────┐
│ Files Tab       │  │ Editor Tab      │   │ Test Tab     │
│                 │  │                 │   │              │
│ [Create]────────┼─→│ Write Python    │   │ Run Contract │
│                 │  │ Click [Save]    │   │              │
└─────────────────┘  └─────────────────┘   └──────────────┘
                                                 ↓
                                           No errors?
                                                 ↓
Step 5: MANAGE          Step 4: DEPLOY        [YES]
┌─────────────────┐    ┌──────────────────┐   ↓
│ Contracts Tab   │   │ Deploy Tab       │  Go to Deploy
│                 │   │                  │   Click Deploy
│ View deployed   │←──│ Click Deploy     │   ↓
│ contracts       │   │ Get address      │   Live!
└─────────────────┘   └──────────────────┘
```

---

## 📁 FILE OPERATIONS

### **Create New File**
```
[Files] Tab
├─ Type: "MyContract"
├─ Click: [Create]
└─ Result: MyContract.py created with template
```

### **Edit File**
```
[Files] Tab [Open] button
└─ [Editor] Tab (auto-opens)
   └─ Edit code
   └─ Click [Save]
```

### **Delete File**
```
[Files] Tab [Delete] button
└─ File removed
```

---

## 🧪 TEST WORKFLOW

### **Run Initialize**
```
[Test] Tab
└─ [Run Contract] button
   ├─ Executes: initialize()
   ├─ Shows: Output
   ├─ Gas: Calculated
   └─ Status: ✓ Success or ❌ Error
```

### **Run Custom Function**
```
[Test] Tab
├─ Type function: "increment"
├─ [Run Function] button
└─ Shows: Result, Gas, Output
```

### **Analyze Code**
```
[Test] Tab
├─ [Analyze Code] button
├─ Shows:
│  ├─ ✓ Code valid
│  ├─ ✓ Class found
│  └─ ✓ Methods found
└─ ⚠️ Warnings (if any)
```

---

## 🎨 COLOR MEANINGS

| Color | Meaning | Action |
|-------|---------|--------|
| ❌ RED | **Error** | **FIX IT** - Don't deploy |
| ⚠️ YELLOW | **Warning** | **REVIEW** - Can still deploy |
| ✓ GREEN | **Success** | **READY** - Can deploy |
| 📊 BLUE | **Info** | **Read** - Status info |
| ⛽ ORANGE | **Gas** | **Track** - Execution cost |

---

## 📋 FILE BROWSER TABLE

```
┌──────────────────────────────────────────────────────────┐
│ Filename | Size | Modified | Action | Delete             │
├──────────────────────────────────────────────────────────┤
│ Filename │ Size │ Date/Time│ [Open] │ [Delete]           │
│ Filename │ Size │ Date/Time│ [Open] │ [Delete]           │
└──────────────────────────────────────────────────────────┘
```

Click [Open] to load into Editor
Click [Delete] to remove file

---

## 🚀 10-MINUTE QUICKSTART

### **Minute 1-2: Setup**
```
1. Launch AxionDigitaverse.exe
2. Register account
3. Login
```

### **Minute 3-4: Create**
```
4. Click ⚙️ IDE
5. Click [Files] tab
6. Type: "hello"
7. Click [Create]
```

### **Minute 5-6: Edit**
```
8. Auto-opens [Editor]
9. Edit the code
10. Click [Save]
```

### **Minute 7-8: Test**
```
11. Click [Test] tab
12. Click [Run Contract]
13. See output ✓
```

### **Minute 9-10: Deploy**
```
14. Click [Deploy] tab
15. Click [Deploy]
16. Contract live! 🎉
```

---

## ⌨️ KEYBOARD SHORTCUTS

```
Ctrl+S   = Save current file
Tab      = Indent code
Enter    = New line
Ctrl+A   = Select all
Ctrl+C   = Copy
Ctrl+V   = Paste
```

---

## 🆘 TROUBLESHOOTING

| Issue | Solution |
|-------|----------|
| File won't save | Check [Files] tab, click Save button |
| Code won't test | Check for errors (❌ RED), fix them |
| Can't find file | Go to [Files] tab, refresh list |
| Deployment failed | Check gas, test first, check errors |
| No contracts show | Go to [Contracts] tab, wait to load |

---

## 📊 TEST OUTPUT GUIDE

### **Output Window Shows**

```
❌ ERRORS:
   Must fix before deploying
   
⚠️ WARNINGS:
   Optional, good to review
   
✓ OUTPUT:
   Execution results
   
📊 RESULT:
   Function return value
   
⛽ Gas Used: XXXX
   Execution cost
```

---

## 🎯 SUCCESS CHECKLIST

Before deploying, verify:

- [ ] File created and saved
- [ ] Code has no ❌ RED errors
- [ ] Analysis shows ✓ valid
- [ ] Test output shows ✓ success
- [ ] Gas usage is reasonable
- [ ] No critical ⚠️ warnings
- [ ] Function works in test
- [ ] Ready to deploy!

---

## 🌟 PRO TIPS

1. **Save Often** - Use Ctrl+S or click Save
2. **Test First** - Always test before deploying
3. **Check Errors** - Fix RED errors immediately
4. **Review Gas** - Monitor gas usage
5. **Use Templates** - Load templates for quick start
6. **Analyze Code** - Use analysis for quality checks
7. **Keep Files** - Organize your contracts
8. **Deploy Ready** - Only deploy when ✓ GREEN

---

## 📈 WORKFLOW SUMMARY

```
START
 ↓
[Create File] → [Edit Code] → [Save] → [Test]
                                          ↓
                                    All Tests Pass?
                                      ↙        ↘
                                    NO        YES
                                    ↓         ↓
                            [Fix Errors]  [Deploy]
                                    ↓         ↓
                                 [Test]   [Done!]
                                    ↓
                            (Loop until YES)
```

---

## 📁 FILES LOCATION

```
All your files saved in:
./axion_contracts/

Examples:
./axion_contracts/counter.py
./axion_contracts/token.py
./axion_contracts/auction.py
```

---

## 🎊 FEATURES AT A GLANCE

✅ **File Management**
- Create, save, open, delete files
- File browser with details
- Auto .py extension

✅ **Code Editor**
- Python syntax
- 65KB capacity
- Auto-save tracking

✅ **Test Environment**
- Run contracts
- Analyze code
- Track gas
- Color-coded errors

✅ **Deployment**
- One-click deploy
- Get address
- Track status

✅ **Management**
- View contracts
- Check state
- Monitor events

---

## 💬 QUICK HELP

**"How do I create a file?"**
→ [Files] tab, type name, click Create

**"Where is my file?"**
→ [Files] tab, see in table

**"How do I test?"**
→ [Test] tab, click Run Contract

**"What does RED mean?"**
→ Error! Must fix before deploying

**"How do I deploy?"**
→ [Deploy] tab, click Deploy button

**"Where are files saved?"**
→ ./axion_contracts/ folder

---

## 🚀 YOU'RE READY!

```
✅ IDE Features: COMPLETE
✅ File Management: READY
✅ Test Environment: WORKING
✅ Deployment: FUNCTIONAL
✅ Documentation: COMPLETE

NOW GO BUILD SMART CONTRACTS! 🚀
```

---

**Axion IDE - Your Smart Contract Development Companion** 🎉

*Professional. Powerful. Production-Ready.*
