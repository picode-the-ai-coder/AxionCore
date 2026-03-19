# 🎯 AXION IDE TEST ENVIRONMENT - QUICK GUIDE

## **How to Test Your Smart Contract Before Deploying**

---

## **Step-by-Step Guide**

### **1️⃣ Write Your Contract**

Go to **Editor** tab and write your Python contract:

```python
class Counter:
    def initialize(self):
        self.count = 0
        return {"status": "initialized"}
    
    def increment(self):
        self.count += 1
        return {"count": self.count}
```

---

### **2️⃣ Click "Test" Tab**

```
[Editor] ► [Test] ◄ [Contracts] [Deploy]...
```

You'll see the Test Environment

---

### **3️⃣ Run Contract Test**

Click: **"Run Contract (Initialize)"** button

You'll see:
```
✓ OUTPUT:
Contract executed
Gas used: 1500

✓ RESULT:
initialized
```

---

### **4️⃣ Check for Errors**

The system shows:

| Color | Meaning | Action |
|-------|---------|--------|
| ❌ RED | Error | FIX it |
| ⚠️ YELLOW | Warning | REVIEW it |
| ✓ GREEN | Success | DEPLOY it |

---

### **5️⃣ Analyze Code**

Click: **"Analyze Code"** button

Get detailed report:
- ✓ Class definition found
- ✓ Methods found
- ✓ No syntax errors
- ⚠️ Missing initialize() [only if missing]

---

### **6️⃣ Test Functions**

1. Type function name: `increment`
2. Click: **"Run Function"**
3. See result:
```
✓ OUTPUT:
Function executed
count = 1

⛽ Gas Used: 800
```

---

### **7️⃣ No Errors? Deploy!**

When you see: **"✓ Ready to deploy!"**

1. Go to **Deploy** tab
2. Click **Deploy**
3. Contract goes live!

---

## **What Gets Tested**

### ✅ Automatically Checked

- [ ] Class definition exists
- [ ] Methods are defined
- [ ] Python syntax is valid
- [ ] Indentation is correct
- [ ] Contract structure is sound
- [ ] No common errors

### ✅ Execution Tested

- [ ] initialize() runs
- [ ] Returns valid output
- [ ] State is set correctly
- [ ] Gas is calculated
- [ ] No runtime errors

### ✅ Functions Tested

- [ ] Function exists
- [ ] Function runs
- [ ] Function returns value
- [ ] State changes work
- [ ] Gas tracked

---

## **Error Messages**

### ❌ Errors (RED - Must Fix)

```
✗ Missing class definition
✗ Function not found
✗ Invalid Python syntax
✗ Contract execution failed
```

**ACTION:** Go to Editor, fix code, test again

### ⚠️ Warnings (YELLOW - Good to Know)

```
⚠ No initialize() method
⚠ Using tabs instead of spaces
⚠ Code is very large (>50KB)
⚠ Unusual contract structure
```

**ACTION:** Consider fixing, but can still deploy

### ✓ Success (GREEN - All Good)

```
✓ Code is valid
✓ Contract structure OK
✓ All tests passed
✓ Ready to deploy!
```

**ACTION:** Deploy to blockchain!

---

## **Common Issues & Fixes**

### Issue: "Missing class definition"
**Fix:** Add `class MyContract:` at top

### Issue: "Function not found"
**Fix:** Make sure function is inside class, properly indented

### Issue: "Indentation error"
**Fix:** Use spaces (not tabs), proper Python indentation

### Issue: "Syntax error"
**Fix:** Check Python syntax, use proper quotes, colons, etc.

---

## **Gas Usage**

Gas is estimated automatically:

```
⛽ initialize():  500-2000 gas
⛽ Simple func:   800-1500 gas
⛽ Complex func:  5000-50000 gas
```

**Lower gas = Better performance**

---

## **Example Workflow**

### Counter Contract

```
1. WRITE
   class Counter:
      def initialize(self): ...
      def increment(self): ...

2. TEST
   [Go to Test tab]
   [Click "Run Contract"]
   ✓ Output shows: initialized

3. ANALYZE
   [Click "Analyze Code"]
   ✓ All checks pass

4. RUN FUNCTION
   [Type: increment]
   [Click "Run Function"]
   ✓ Output shows: count = 1

5. DEPLOY
   [No errors? Go to Deploy tab]
   [Click Deploy]
   ✓ Contract live!
```

---

## **Best Practices**

1. ✅ **Always test before deploying**
2. ✅ **Check for errors (RED)**
3. ✅ **Review warnings (YELLOW)**
4. ✅ **Test all functions**
5. ✅ **Check gas usage**
6. ✅ **Verify state changes**
7. ✅ **Only deploy when GREEN**

---

## **Keyboard Shortcuts**

```
Ctrl+N   = New contract
Ctrl+D   = Clear editor
Tab      = Indent code
Enter    = New line
```

---

## **Quick Commands**

| Button | Action | Result |
|--------|--------|--------|
| Run Contract | Execute initialize | See output |
| Run Function | Execute any function | See results |
| Analyze Code | Check code quality | See report |
| Clear Output | Clear results | Fresh start |

---

## **FAQ**

**Q: What if I get an error?**
A: Read the error message, go to Editor, fix code, test again.

**Q: Can I deploy with warnings?**
A: Yes, warnings are just suggestions. Errors must be fixed.

**Q: How do I know gas is correct?**
A: The system estimates based on code complexity. Test to verify.

**Q: Can I test multiple times?**
A: Yes! Test as many times as you want before deploying.

**Q: What if test passes but deploy fails?**
A: Rare, but go back to Test tab, run again, fix any new errors.

---

## **You're Ready!**

```
Your Smart Contract IDE now has:
  ✅ Test Environment
  ✅ Error Detection
  ✅ Code Analysis
  ✅ Gas Tracking
  ✅ Safe Deployment

Start building confident contracts!
```

---

**Next Step:** 

1. Open **Axion Digitaverse**
2. Login
3. Go to **⚙️ IDE** tab
4. Load a template
5. Click **Test** tab
6. Click **"Run Contract"**
7. See it work!

🚀 **Happy coding!** 🚀
